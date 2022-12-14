
#include <iostream>
#include <stdlib.h>
#include <GL/glut.h>
#include <vector>
#include <sstream>
#include "mpi.h"
#include <string>
#include <complex>
#include <time.h>

#define NBITERATIONS 50
double nbPixX=600;
double nbPixY=600;
double nbP = 1.5;
#define NBPIX 600

using namespace std ;
void affichage(void);
int numtasks=0 ;
void clavier(unsigned char touche,int x,int y);
void affiche_repere(void);

void mouse(int, int, int, int);
void mouseMotion(int, int);
//void reshape(int,int);

double depx=0;
double depy=0;

// variables globales pour OpenGL
bool mouseLeftDown;
bool mouseRightDown;
bool mouseMiddleDown;
float mouseX, mouseY;
float cameraAngleX;
float cameraAngleY;
float cameraDistance=1;

int mandel(complex<double> c)
{
    int n=0;
    complex<double> zn(0,0);
    int conv = 0;
    while(n<NBITERATIONS && abs(zn)<=2 && conv<5)
    {
        complex<double> zn1 = pow(zn,2);
        zn1 = zn1 + c;
        n++;
        if(abs(imag(zn)-imag(zn1))<0.0000000001 && abs(real(zn)-real(zn1))<0.0000000001)
        {
            conv+=1;
        }
        zn = zn1;
    }
    if(abs(zn)<=2)
    {
        
        return -1;
    }
    else
    {
        return n;
    }
    return 0;
}


//----------------------------------------------------------------------------------
void initOpenGl()
//----------------------------------------------------------------------------------
{

//lumiere

	glClearColor( .5, .5, 0.5, 0.0 );

	glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat l_pos[] = { 3.,3.5,3.0,1.0 };
    glLightfv(GL_LIGHT0,GL_POSITION,l_pos);

     glLightfv(GL_LIGHT0,GL_DIFFUSE,l_pos);
     glLightfv(GL_LIGHT0,GL_SPECULAR,l_pos);
     glEnable(GL_COLOR_MATERIAL);

     glMatrixMode(GL_PROJECTION);
     glLoadIdentity();
     gluPerspective(45.0f,(GLfloat)nbPixX/(GLfloat)nbPixY,0.1f,100.0f);
	glMatrixMode(GL_MODELVIEW);

    gluLookAt(0.,0.,4., 0.,0.,0., 0.,1.,0.);
  
}


int main(int argc,char **argv)
{

    int  rank, len, rc;
    char hostname[MPI_MAX_PROCESSOR_NAME];


 // initialize MPI
    MPI_Init(&argc,&argv);

    // get number of tasks
    MPI_Comm_size(MPI_COMM_WORLD ,&numtasks);

    // get my rank
     MPI_Comm_rank(MPI_COMM_WORLD ,&rank);

     
 MPI_Get_processor_name(hostname , &len);
 printf ("Number of tasks= %d My rank= %d Running on %s\n", numtasks ,rank,hostname
);
fflush(stdout);

  if(rank==0){

    cout<<"hauteur de la fenetre"<<endl;
    cin>>nbPixY;
    cout<<"largeur de la fenetre"<<endl;
    cin>>nbPixX;
    
  if(nbP<1.5)
  {
    nbP=1.5;
  }
  double dimension[2]{nbPixX,nbPixY};
  MPI_Bcast(&dimension,2,MPI_DOUBLE,0,MPI_COMM_WORLD);
  
  /* initialisation de glut et creation
     de la fenetre */
  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_RGB);
  glutInitWindowPosition(200,200);
  glutInitWindowSize(nbPixX,nbPixY);
  glutCreateWindow("ifs");

  /* Initialisation d'OpenGL */
  glClearColor(0.0,0.0,0.0,0.0);
  glColor3f(1.0,1.0,1.0);
  glPointSize(1.0);

  /* enregistrement des fonctions de rappel */
  glutDisplayFunc(affichage);
  glutKeyboardFunc(clavier);
  glutMouseFunc(mouse);
  glutMotionFunc(mouseMotion);

initOpenGl() ;

/* Entree dans la boucle principale glut */
  glutMainLoop();
  }
else{
    
    double buffer[3];
 MPI_Bcast(&buffer,2,MPI_DOUBLE,0,MPI_COMM_WORLD);
 nbPixX=buffer[0];
 nbPixY=buffer[1];
if(rank==1){
while(true){
    MPI_Bcast(&buffer,3,MPI_DOUBLE,0,MPI_COMM_WORLD);//depx depy et cameradistance
   
  int nombrpixint= nbPixX/(numtasks-2);
   for(int i=2;i<numtasks-1;i++)
   {
    buffer[0]=(i-2)*nombrpixint;
    buffer[1]=(i-1)*nombrpixint; 

    MPI_Send(&buffer,2,MPI_DOUBLE,i,15,MPI_COMM_WORLD); 


   }
    buffer[0]=(numtasks-3)*nombrpixint;
    buffer[1]=nbPixX;
   MPI_Send(&buffer,2,MPI_DOUBLE,numtasks-1,15,MPI_COMM_WORLD); 
}

}
else{
    while(true )
    {
        

        MPI_Bcast(&buffer,3,MPI_DOUBLE,0,MPI_COMM_WORLD);//depx depy et cameradistance
        depx=buffer[0];
         depy=buffer[1];
         cameraDistance=buffer[2];
        //mpi receive en provenance de 1 puis traite les données et les envoies a 0
        MPI_Recv(&buffer,2,MPI_DOUBLE,1,15,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
       double imin=buffer[0];
       double imax=buffer[1];
       int taby[(int)((imax-imin+1)*nbPixY)+1];
       int icr=1;
        for(int i=imin;i<imax;i++)
        {
            
            for(int j=-nbPixY/2; j<nbPixY/2;j++)
            {
                
                double v = ((double)i-nbPixX/2)/(nbPixX/4);
                double w = (double)j/(nbPixY/4);
                complex<double> x(v/(double)cameraDistance+(double)depx/(100.0),w/(double)cameraDistance-(double)depy/(100.0));
                
                //fait pour calculer la vitesse de calcule et de dessins  
                
                int y = 1;
                complex<double> t1(real(x)+0.24,imag(x));
                complex<double> t2(real(x),imag(x)+0.3);
                complex<double> t3(real(x),imag(x)-0.3);
                complex<double> t4(real(x)+1,imag(x));

                if(abs(t1)<=0.49 || abs(t2)<=0.3 || abs(t3)<=0.3 || abs(t4)<=0.23)
                {
                    y=-2;
                }
                else
                {
                    if(abs(x)<=2)
                    {
                        y = mandel(x);
                    }
                }
                taby[icr]=y;
                icr++;
            }
        }
        taby[0]=icr;
        MPI_Send(&taby,icr,MPI_INT,0,12,MPI_COMM_WORLD);
    }
}


}

  MPI_Finalize();
  return 0;
}
//------------------------------------------------------


void affichage(void)
{
    double tab[3]{depx,depy,cameraDistance};
    MPI_Bcast(&tab,3,MPI_DOUBLE,0,MPI_COMM_WORLD);//depx depy et cameradistance
    double st1 = 0;
    double st2 = 0;
	glMatrixMode(GL_MODELVIEW);
	glClear(GL_COLOR_BUFFER_BIT);
	glPushMatrix();

    glPointSize(nbP);
    glBegin(GL_POINTS);
    int countx=0;
    int county=0;
        for(int i=2;i<numtasks;i++)
            {    
                int taillebuffer=(nbPixX/(numtasks-2)+(int)nbPixX%(numtasks-2))*(int)nbPixY;
                int buffer[taillebuffer+1];
                MPI_Recv(&buffer,taillebuffer+1,MPI_INT,i,12,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                
                for(int z=1;z<buffer[0];z++){
                   int y=buffer[z];
                   
                   double v = ((double)countx-nbPixX/2)/(nbPixX/4);
                double w = ((double)county-nbPixY/2)/(nbPixY/4);
                        if(y==-1)
                        {
                            glColor3f(0,0,0);
                        }
                        else
                        {
                            if(y==-2)
                            {
                                glColor3f(1,1,1);
                            }
                            else
                                glColor3f(1*(double)y/(double)NBITERATIONS,1*(double)y/(double)NBITERATIONS,0.1);
                        }
                        
                        glVertex2f(v,w);
                        clock_t end2 = clock();
                        county++;
                        
                        if(county==nbPixY){countx++;county=0;}
                }
            }
    
    glEnd();
    glPopMatrix();
  /* on force l'affichage du resultat */
  glFlush();
}

//------------------------------------------------------


//------------------------------------------------------
void clavier(unsigned char touche,int x,int y)
{

  switch (touche)
    {
    case '+':
      glutPostRedisplay();
      break;
    case '-':
      glutPostRedisplay();
      break;
    case 'f': //* affichage en mode fil de fer
      glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
      glutPostRedisplay();
      break;
      case 'p': //* affichage du carre plein
      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      glutPostRedisplay();
      break;
  case 's' : //* Affichage en mode sommets seuls
      glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
      glutPostRedisplay();
      break;
    case 'q' : //*la touche 'q' permet de quitter le programme
    //MPI_Send();// arret a 1 
    MPI_Finalize();
      exit(0); //envoie un message a tous le monde de terminer tous les autres processus 0 envoie a 1 de terminer tout les autres process
    }

}
void mouse(int button, int state, int x, int y)
{
    mouseX = x;
    mouseY = y;

    if(button == GLUT_LEFT_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseLeftDown = true;
        }
        else if(state == GLUT_UP)
            mouseLeftDown = false;
    }

    else if(button == GLUT_RIGHT_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseRightDown = true;
        }
        else if(state == GLUT_UP)
            mouseRightDown = false;
    }

    else if(button == GLUT_MIDDLE_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseMiddleDown = true;
        }
        else if(state == GLUT_UP)
            mouseMiddleDown = false;
    }
}


void mouseMotion(int x, int y)
{
    if(mouseLeftDown)
    {
        //cameraAngleY += (x - mouseX);
        //cameraAngleX += (y - mouseY);
        depx += (x-mouseX)/(double)cameraDistance;
        depy += (y-mouseY)/(double)cameraDistance;

        /*

        ATTENTION : IMPORTANT
        CONTROLER LES DEPLACEMENTS

        */

        if(depx<100*(-2.5+2/cameraDistance))
            depx=100*(-2.5+2/cameraDistance);
        if(depx>100*(2.5-2/cameraDistance))
            depx=100*(2.5-2/cameraDistance);

        if(depy<100*(-2.5+2/cameraDistance))
            depy=100*(-2.5+2/cameraDistance);
        if(depy>100*(2.5-2/cameraDistance))
            depy=100*(2.5-2/cameraDistance);

        mouseX = x;
        mouseY = y;
    }
    if(mouseRightDown)
    {
        cameraDistance += (y - mouseY) * 0.01f;
        if(cameraDistance<1)
        {
            cameraDistance=1;
        }

        /*

        ATTENTION : IMPORTANT
        CONTROLER LES DEPLACEMENTS

        */

        if(depx<100*(-2.5+2/cameraDistance))
            depx=100*(-2.5+2/cameraDistance);
        if(depx>100*(2.5-2/cameraDistance))
            depx=100*(2.5-2/cameraDistance);

        if(depy<100*(-2.5+2/cameraDistance))
            depy=100*(-2.5+2/cameraDistance);
        if(depy>100*(2.5-2/cameraDistance))
            depy=100*(2.5-2/cameraDistance);

        mouseY = y;
    }

    glutPostRedisplay();
}
