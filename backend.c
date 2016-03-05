#include <stdio.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv.h>
#include <gsl/gsl_odeiv2.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#define PI 3.14159265358979323846
FILE *output1,*output2,*pipe_gp,*dots;
double u=1.0;
double alfa1=(90.0*PI)/180.0;
double alfa2=(90.0*PI)/180.0;
double w1=0.0;
double w2=0.0;
int pid;
void sig_usr1(int signo){
  exit(0);
}
int func (double t, const double yin[], double f[],void *params[]){

  double M1=1.0;
  double M2=M1*u;
  double G=9.81;
  double L1=1.0;
  double L2=1.0;

  float den1, den2, del;

  f[0] = yin[1];

  del = yin[2]-yin[0];
  den1 = (M1+M2)*L1 - M2*L1*cos(del)*cos(del);
  f[1] = (M2*L1*yin[1]*yin[1]*sin(del)*cos(del)
    + M2*G*sin(yin[2])*cos(del) + M2*L2*yin[3]*yin[3]*sin(del)
    - (M1+M2)*G*sin(yin[0]))/den1;

  f[2] = yin[3];

  den2 = (L2/L1)*den1;
  f[3] = (-M2*L2*yin[3]*yin[3]*sin(del)*cos(del)
    + (M1+M2)*G*sin(yin[0])*cos(del)
    - (M1+M2)*L1*yin[1]*yin[1]*sin(del)
    - (M1+M2)*G*sin(yin[2]))/den2;

  return GSL_SUCCESS;
}

int main (int argc,char *argv[]){

  double x1,x2,y1,y2;

  pid=atoi(argv[1]);
  alfa1=alfa2=(atoi(argv[2])*PI)/180.0;
  u=atof(argv[3]);
  FILE *dots;
  dots=fopen("dots.txt","w");
  fclose(dots);
  gsl_odeiv2_system sys = { func, NULL, 4, NULL };


  gsl_odeiv2_driver *d = gsl_odeiv2_driver_alloc_y_new (&sys, gsl_odeiv2_step_rk4,1e-3, 1e-3, 1e-3);

  struct sigaction act1;
  act1.sa_handler=sig_usr1;
  sigemptyset(&act1.sa_mask);
  sigaddset(&act1.sa_mask,SIGUSR1);
  act1.sa_flags=0;

  if(sigaction(SIGUSR1,&act1,NULL) == -1){
    perror("can't catch SIGUSR1");
    exit(1);
  }
  dots=fopen("dots.txt","w");
  fclose(dots);
  double t = 0.0;
  double y[4] = { alfa1 , w1,alfa2,w2 };
  int s;
  while(1){
  //for (i = 0; i < 100; i++){
    if(kill(pid,0)<0){
      exit(1);
    }

    if((dots=fopen("dots.txt","a"))==NULL){
      printf("dots open error\n");
      exit(1);
    }
    if((output1=fopen("data1.txt","w"))==NULL){
      printf("data1 open error\n");
      exit(1);
    }
    if((output2=fopen("data2.txt","w"))==NULL){
      printf("data2 open error\n");
      exit(1);
    }
    s = gsl_odeiv2_driver_apply_fixed_step (d, &t, 1e-3, 40, y);

    if (s != GSL_SUCCESS){
        printf ("error: driver returned %d\n", s);
        break;
    }
    x1=sin(y[0]);
    y1=-cos(y[0]);
    x2=x1+sin(y[2]);
    y2=y1-cos(y[2]);
    if(fprintf(output1,"0.0 0.0\n")<0){
      printf("printf error\n");
      exit(1);
    }
    if(fprintf(output1,"%f %f\n",x1,y1)<0){
      printf("printf error\n");
      exit(1);
    }
    if(fprintf(output2,"%f %f\n",x1,y1)<0){
      printf("printf error\n");
      exit(1);
    }
    if(fprintf(output2,"%f %f\n",x2,y2)<0){
      printf("printf error\n");
      exit(1);
    }
    if(fprintf(dots,"%f %f\n",x2,y2)<0){
      printf("printf error\n");
      exit(1);
    }
    if((pipe_gp = popen("gnuplot", "w"))==NULL){
      printf("gnuplot open error\n");
      exit(1);
    }

    if(fprintf(pipe_gp," load \"pendulum_plot\"\n")<0){
      printf("printf error\n");
      exit(1);
    }
    if(fclose(output1)<0){
      printf("output1 close error\n");
      exit(1);
    }
    if(fclose(output2)<0){
      printf("output2 close error\n");
      exit(1);
    }
    if(fclose(dots)<0){
      printf("odots close error\n");
      exit(1);
    }
    if(pclose(pipe_gp)<0){
      printf("gnuplot close error\n");
      exit(1);
    }
    if(kill(pid,SIGUSR1)<0){
      printf("kill error\n");
      exit(1);
    }
    usleep(12000);

  }
  gsl_odeiv2_driver_free (d);
  return s;
}
