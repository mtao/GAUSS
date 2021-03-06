#include <functional>

#include <Qt3DIncludes.h>
#include <GaussIncludes.h>
#include <FEMIncludes.h>

//Any extra things I need such as constraints
#include <ConstraintFixedPoint.h>
#include <TimeStepperEigenFitSMW.h>
#include <EigenFit.h>
#include <fstream>

using namespace Gauss;
using namespace FEM;
using namespace ParticleSystem; //For Force Spring

/* Tetrahedral finite elements */

//typedef physical entities I need

//typedef scene
typedef PhysicalSystemFEM<double, NeohookeanHFixedTet> FEMLinearTets;

typedef World<double, std::tuple<FEMLinearTets *>,
std::tuple<ForceSpringFEMParticle<double> *, ForceParticlesGravity<double> *>,
std::tuple<ConstraintFixedPoint<double> *> > MyWorld;

//typedef World<double, std::tuple<FEMLinearTets *,PhysicalSystemParticleSingle<double> *>,
//                      std::tuple<ForceSpringFEMParticle<double> *>,
//                      std::tuple<ConstraintFixedPoint<double> *> > MyWorld;
//typedef TimeStepperEigenFitSMW<double, AssemblerEigenSparseMatrix<double>, AssemblerEigenVector<double>> MyTimeStepper;
typedef TimeStepperEigenFitSMW<double, AssemblerParallel<double, AssemblerEigenSparseMatrix<double>>, AssemblerParallel<double, AssemblerEigenVector<double>> > MyTimeStepper;

typedef Scene<MyWorld, MyTimeStepper> MyScene;


//typedef TimeStepperEigenFitSI<double, AssemblerParallel<double, AssemblerEigenSparseMatrix<double> >,
//AssemblerParallel<double, AssemblerEigenVector<double> > > MyTimeStepper;

//typedef Scene<MyWorld, MyTimeStepper> MyScene;

// used for preStepCallback. should be delete
std::vector<ConstraintFixedPoint<double> *> movingConstraints;
Eigen::VectorXi movingVerts;
Eigen::MatrixXd V;
Eigen::MatrixXi F;
char **arg_list;
unsigned int istep;

void preStepCallback(MyWorld &world) {
//    // This is an example callback
//    if (atoi(arg_list[5]) == 2)
//    {
//        // This is an example callback
//        
//        //script some motion
////
//        if (istep < 50) {
//            
//            for(unsigned int jj=0; jj<movingConstraints.size(); ++jj) {
//                if(movingConstraints[jj]->getImpl().getFixedPoint()[0] > -3) {
//                    Eigen::Vector3d v = V.row(movingVerts[jj]);
//                    Eigen::Vector3d new_p = v + Eigen::Vector3d(0.0,1.0/10,0.0);
//                    movingConstraints[jj]->getImpl().setFixedPoint(new_p);
//                }
//            }
//        }
//    }
}

int main(int argc, char **argv) {
    //    arg list
    //    1: full path to coarse mesh
    //    2: full path to fine mesh
    //    3: youngs modulus (SI unit)
    //    4: constraint threshold (for defualt constraint profile)
    //    5: constraint profile switch
    //    6: name of the initial deformation profile
    //    7. number of time steps
//    8. flag for using hausdorff distance
//    9. number of modes to modifies
    std::cout<<"Test Neohookean FEM EigenFit\n";
    
    //Setup Physics
    MyWorld world;
    
    arg_list = argv;
    
    Eigen::MatrixXd Vf;
    Eigen::MatrixXi Ff;
    
    
    
    //    define the file separator base on system
    const char kPathSeparator =
#ifdef _WIN32
    '\\';
#else
    '/';
#endif
    
    if (argc > 1) {
        // must supply all 9 parameters
        
        std::string cmeshname = argv[1];
        std::string fmeshname = argv[2];
        
        readTetgen(V, F, dataDir()+cmeshname+".node", dataDir()+cmeshname+".ele");
        readTetgen(Vf, Ff, dataDir()+fmeshname+".node", dataDir()+fmeshname+".ele");
        
        std::string::size_type found = cmeshname.find_last_of(kPathSeparator);
        //    acutal name for the mesh, no path
        std::string cmeshnameActual = cmeshname.substr(found+1);

        //    acutal name for the mesh, no path
        std::string fmeshnameActual = fmeshname.substr(found+1);

        
        //    parameters
        double youngs = atof(argv[3]);
        double poisson = 0.45;
        int constraint_dir = 0; // constraint direction. 0 for x, 1 for y, 2 for z
        double constraint_tol = atof(argv[4]);
        //
        // send the constraint switch in as well, or the fine embedded mesh. ugly
        // the flag indicate whether to recalculated or not
        // need to pass the material and constraint parameters to embedding too. need to do it again below. ugly
        // also use the last two args to determine how many modes to fix. have to put it here now. ugly
        EigenFit *test = new EigenFit(V,F,Vf,Ff,true,youngs,poisson,constraint_dir,constraint_tol, atoi(argv[5]),atoi(argv[8]),atoi(argv[9]));
        
        
        world.addSystem(test);
        

        // projection matrix for constraints
        Eigen::SparseMatrix<double> P;
        if (atoi(argv[5]) == 0) {
            // constraint switch
            
            //            zero gravity
            Eigen::Vector3x<double> g;
            g(0) = 0;
            g(1) = 0;
            g(2) = 0;
            
            for(unsigned int iel=0; iel<test->getImpl().getF().rows(); ++iel) {
                
                test->getImpl().getElement(iel)->setGravity(g);
                
            }
            
            world.finalize(); //After this all we're ready to go (clean up the interface a bit later)
            
            //            set the projection matrix to identity because there is no constraint to project
//            Eigen::SparseMatrix<double> P;
            P.resize(V.rows()*3,V.rows()*3);
            P.setIdentity();
//            std::cout<<P.rows();
            //            no constraints
        }
        else if(atoi(argv[5]) == 1)
        {
            //    default constraint
            fixDisplacementMin(world, test,constraint_dir,constraint_tol);
            world.finalize(); //After this all we're ready to go (clean up the interface a bit later)
            
            // construct the projection matrix for stepper
            Eigen::VectorXi indices = minVertices(test, constraint_dir,constraint_tol);
            P = fixedPointProjectionMatrix(indices, *test,world);
            
        }
        else if (atoi(argv[5]) == 2)
        {
            

            movingVerts = minVertices(test, constraint_dir, constraint_tol);//indices for moving parts
//
            for(unsigned int ii=0; ii<movingVerts.rows(); ++ii) {
                movingConstraints.push_back(new ConstraintFixedPoint<double>(&test->getQ()[movingVerts[ii]], Eigen::Vector3d(0,0,0)));
                world.addConstraint(movingConstraints[ii]);
            }
            fixDisplacementMin(world, test,constraint_dir,constraint_tol);
            
            world.finalize(); //After this all we're ready to go (clean up the interface a bit later)
            
            P = fixedPointProjectionMatrix(movingVerts, *test,world);
            
        }
        
        // set material
        for(unsigned int iel=0; iel<test->getImpl().getF().rows(); ++iel) {
            
            test->getImpl().getElement(iel)->setParameters(youngs, poisson);
            
        }
        
        auto q = mapStateEigen(world);
        auto fine_q = mapStateEigen(test->getFineWorld());
        //    default to zero deformation
        q.setZero();

        if (strcmp(argv[6],"0")==0) {
            
            q.setZero();
        }
        else
        {
            
            std::string qfileName(argv[6]);
            Eigen::VectorXd  tempv;
            loadMarketVector(tempv,qfileName);

            q = tempv;
            
        }
        
        MyTimeStepper stepper(0.01,P);
        
        //         the number of steps to take
        
        unsigned int file_ind = 0;
        std::string name = "pos";
        std::string fformat = ".obj";
        std::string filename = name + std::to_string(file_ind) + fformat;
        std::string qname = cmeshnameActual + "ExampleQ";
        std::string qfformat = ".mtx";
        std::string qfilename = qname + std::to_string(file_ind) + qfformat;
        std::string qname2 = fmeshnameActual + "ExampleQ";
        std::string qfilename2 = qname2 + std::to_string(file_ind) + qfformat;
        
        struct stat buf;
        unsigned int idx;
        
        for(istep=0; istep<atoi(argv[7]) ; ++istep) {
            stepper.step(world);
            
            // acts like the "callback" block
            if (atoi(arg_list[5]) == 2)
            {
                //script some motion
                //
                
                
                for(unsigned int jj=0; jj<movingConstraints.size(); ++jj) {
                    
                    auto v_q = mapDOFEigen(movingConstraints[jj]->getDOF(0), world.getState());
//
//                    if ((istep%150) < 50) {
//                        Eigen::Vector3d new_q = (istep%150)*Eigen::Vector3d(0.0,-1.0/100,0.0);
//                        v_q = new_q;
//                    }
//                    else if ((istep%150) < 100)
//                    {}
//                    else
//                    {
//                        Eigen::Vector3d new_q =  (150-(istep%150))*Eigen::Vector3d(0.0,-1.0/100,0.0);
//                        v_q = new_q;
//                    }
                    Eigen::Vector3d new_q = (istep)*Eigen::Vector3d(0.0,-1.0/100,0.0);
                    v_q = new_q;

                }
            }
            
            //output data here
            std::ofstream ofile;
            ofile.open("KE.txt", std::ios::app); //app is append which means it will put the text at the end
            ofile << std::get<0>(world.getSystemList().getStorage())[0]->getImpl().getKineticEnergy(world.getState()) << std::endl;
            ofile.close();
            
            while (stat(filename.c_str(), &buf) != -1)
            {
                file_ind++;
                filename = name + std::to_string(file_ind) + fformat;
                qfilename = qname + std::to_string(file_ind) + qfformat;
                qfilename2 = qname2 + std::to_string(file_ind) + qfformat;
                
            }
            
            idx = 0;
            // getGeometry().first is V
            Eigen::MatrixXd V_disp = std::get<0>(world.getSystemList().getStorage())[0]->getGeometry().first;
            
            for(unsigned int vertexId=0;  vertexId < std::get<0>(world.getSystemList().getStorage())[0]->getGeometry().first.rows(); ++vertexId) {
                
                // because getFinePosition is in EigenFit, not another physical system Impl, so don't need getImpl()
                V_disp(vertexId,0) += q(idx);
                idx++;
                V_disp(vertexId,1) += q(idx);
                idx++;
                V_disp(vertexId,2) += q(idx);
                idx++;
            }
            igl::writeOBJ(filename,V_disp,std::get<0>(world.getSystemList().getStorage())[0]->getGeometry().second);
            // coarse mesh data
            q = mapStateEigen(world);
            saveMarketVector(q, qfilename);
            // fine mesh data from embedded mesh in eigenfit
            fine_q = mapStateEigen(test->getFineWorld());
            saveMarketVector(fine_q, qfilename2);
        }
    }
    else
    {
        // using all default paramters for eigenfit
     
        //    default example meshes
        std::string cmeshname = "/meshesTetgen/arma/arma_6";
        std::string fmeshname = "/meshesTetgen/arma/arma_1";
        
        readTetgen(V, F, dataDir()+cmeshname+".node", dataDir()+cmeshname+".ele");
        readTetgen(Vf, Ff, dataDir()+fmeshname+".node", dataDir()+fmeshname+".ele");
        
        std::string::size_type found = cmeshname.find_last_of(kPathSeparator);
        //    acutal name for the mesh, no path
        std::string cmeshnameActual = cmeshname.substr(found+1);
        
        
        //    default parameters
        double youngs = 5e5;
        double poisson = 0.45;
        int constraint_dir = 0; // constraint direction. 0 for x, 1 for y, 2 for z
        double constraint_tol = 2e-1;
        
        // no constraint switch so just create the eigenfit obj with constraint switch set to 1
        // the flag indicate whether to recalculated or not
        // need to pass the material and constraint parameters to embedding too. need to do it again below. ugly
        // also use the last two args to determine how many modes to fix. default not using hausdorff distance, and use 10 modes. have to put it here now.  ugly
        EigenFit *test = new EigenFit(V,F,Vf,Ff,true,youngs,poisson,constraint_dir,constraint_tol, 1,false,10);
        
        // set material
        for(unsigned int iel=0; iel<test->getImpl().getF().rows(); ++iel) {
            
            test->getImpl().getElement(iel)->setParameters(youngs, poisson);
            
        }
        
        world.addSystem(test);
        
        world.finalize(); //After this all we're ready to go (clean up the interface a bit later)
        // IMPORTANT, need to finalized before fix boundary
        
        //    default constraint
        fixDisplacementMin(world, test,constraint_dir,constraint_tol);
        
        // construct the projection matrix for stepper
        Eigen::VectorXi indices = minVertices(test, constraint_dir,constraint_tol);
        Eigen::SparseMatrix<double> P = fixedPointProjectionMatrix(indices, *test,world);
        
        
        auto q = mapStateEigen(world);
        
        //    default to zero deformation
        q.setZero();
        

        MyTimeStepper stepper(0.01,P);
        
        //Display
        QGuiApplication app(argc, argv);
        
        MyScene *scene = new MyScene(&world, &stepper, preStepCallback);
        GAUSSVIEW(scene);
        
        return app.exec();
        
    }
    
}
