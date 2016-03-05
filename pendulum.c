#include <gtk/gtk.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>
#define PI 3.14159265358979323846
GtkWidget *image;
int calculate_pid;
int angle=110;
char buf[5];
double proportion=1.0;
typedef struct gtkobject {
  GtkWidget *angle_text;
  GtkWidget *image;
  GtkWidget *proportion_text;
}myObject;
void start_print(){
  FILE *output1,*output2,*pipe_gp,*dots;
  double x1,x2,y1,y2;
  if((output1=fopen("data1.txt","w"))==NULL){
    printf("data1 open error\n");
    exit(1);
  }
  if((output2=fopen("data2.txt","w"))==NULL){
    printf("data2 open error\n");
    exit(1);
  }
  if((dots=fopen("dots.txt","w"))==NULL){
    printf("data2 open error\n");
    exit(1);
  }
  if((pipe_gp = popen("gnuplot", "w"))==NULL){
    printf("gnuplot open error\n");
    exit(1);
  }
  fprintf(dots,"0 0\n");
  x1=sin(angle*PI/180);
  y1=-cos(angle*PI/180);
  x2=x1+sin(angle*PI/180);
  y2=y1-cos(angle*PI/180);
  gtk_image_set_from_file( GTK_IMAGE(image), "pendulum.png" );
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

  gtk_image_set_from_file( GTK_IMAGE(image), "pendulum.png" );
}
void quitf(GtkWidget *widget, myObject *object){
    if(kill(calculate_pid,SIGUSR1)<0){
      printf("kill error\n");
      exit(1);
    }
    exit(0);
}
void inc_angle(GtkWidget *widget, myObject *object){
  if(angle<360){
    angle++;
    sprintf(buf, "%d", angle);
    start_print();
    gtk_entry_set_text(GTK_ENTRY(object->angle_text),buf);
  }
}
void dec_angle(GtkWidget *widget, myObject *object){
  if(angle>0){
    angle--;
    sprintf(buf, "%d", angle);
    start_print();
    gtk_entry_set_text(GTK_ENTRY(object->angle_text),buf);
  }
}
void inc_proportion(GtkWidget *widget, myObject *object){
  proportion+=0.1;
  sprintf(buf, "%1.1f", proportion);
  gtk_entry_set_text(GTK_ENTRY(object->proportion_text),buf);
}
void dec_proportion(GtkWidget *widget, myObject *object){
  if(proportion>0){
    proportion-=0.1;
  }
  sprintf(buf, "%1.1f", proportion);
  gtk_entry_set_text(GTK_ENTRY(object->proportion_text),buf);
}
void calculate(GtkWidget *widget, myObject *object){
  angle=atoi(gtk_entry_get_text (GTK_ENTRY(object->angle_text)));
  proportion=atof(gtk_entry_get_text (GTK_ENTRY(object->proportion_text)));
  pid_t pid;
  if((pid=vfork())<0){
    printf("fork error\n");
    exit(1);
  }
  else if(pid==0){
    calculate_pid=(int)getpid();
    int pid=(int)getppid();
    char cpid[5];
    char cangle[5];
    char cproportion[5];
    sprintf(cpid,"%d",pid);
    sprintf(cangle,"%d",angle);
    sprintf(cproportion,"%1.1f",proportion);
    execlp("./backend","backend",cpid,cangle,cproportion,NULL);
  }
  //wait(NULL);
  gtk_image_set_from_file( GTK_IMAGE(object->image), "pendulum.png" );
}
static void sig_usr1(int signo){
  gtk_image_set_from_file( GTK_IMAGE(image), "pendulum.png" );
}
void stop_calculate(GtkWidget *widget, myObject *object){
  if(kill(calculate_pid,SIGUSR1)<0){
    printf("kill error\n");
    exit(1);
  }
}
int main(int argc,char *argv[]){

  struct sigaction act1;
  act1.sa_handler=sig_usr1;
  sigemptyset(&act1.sa_mask);
  sigaddset(&act1.sa_mask,SIGUSR1);
  act1.sa_flags=0;

  if(sigaction(SIGUSR1,&act1,NULL) == -1){
    perror("can't catch SIGUSR1");
    exit(1);
  }

  GtkWidget *window;
  GtkWidget *frame;
  GtkWidget *on;
  GtkWidget *stop;
  GtkWidget *angle_inc;
  GtkWidget *angle_dec;
  GtkWidget *angle_text;
  GtkWidget *proportion_inc;
  GtkWidget *proportion_dec;
  GtkWidget *proportion_text;


  gtk_disable_setlocale();

  gtk_init(&argc, &argv);
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 600, 450);

  frame = gtk_fixed_new();
  gtk_container_add(GTK_CONTAINER(window), frame);

  image=gtk_image_new_from_file("");
  gtk_fixed_put(GTK_FIXED(frame), image, 0, 35);

  on = gtk_button_new_with_label("go");
  gtk_widget_set_size_request(on, 35, 35);
  gtk_fixed_put(GTK_FIXED(frame), on, 0, 0);

  stop = gtk_button_new_with_label("stop");
  gtk_widget_set_size_request(stop, 35, 35);
  gtk_fixed_put(GTK_FIXED(frame), stop, 35, 0);

  angle_inc = gtk_button_new_with_label("+");
  gtk_widget_set_size_request(angle_inc, 35, 35);
  gtk_fixed_put(GTK_FIXED(frame), angle_inc, 80, 0);

  angle_dec = gtk_button_new_with_label("-");
  gtk_widget_set_size_request(angle_dec, 35, 35);
  gtk_fixed_put(GTK_FIXED(frame), angle_dec, 155, 0);

  angle_text = gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(angle_text),3);
  gtk_fixed_put(GTK_FIXED(frame), angle_text, 115, 0);
  gtk_entry_set_text(GTK_ENTRY(angle_text),"110");

  proportion_inc = gtk_button_new_with_label("+");
  gtk_widget_set_size_request(proportion_inc, 35, 35);
  gtk_fixed_put(GTK_FIXED(frame), proportion_inc, 220, 0);

  proportion_dec = gtk_button_new_with_label("-");
  gtk_widget_set_size_request(proportion_dec, 35, 35);
  gtk_fixed_put(GTK_FIXED(frame), proportion_dec, 295, 0);

  proportion_text = gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(proportion_text),3);
  gtk_fixed_put(GTK_FIXED(frame), proportion_text, 255, 0);
  gtk_entry_set_text(GTK_ENTRY(proportion_text),"1.0");

  myObject object;
  object.image=image;
  object.angle_text=angle_text;
  object.proportion_text=proportion_text;

  start_print("pendulum.png");

  g_signal_connect(window, "destroy",
      G_CALLBACK (quitf), NULL);

  g_signal_connect(stop, "clicked",
      G_CALLBACK(stop_calculate), &object);

  g_signal_connect(on, "clicked",
        G_CALLBACK(calculate),&object );

  g_signal_connect(angle_inc, "clicked",
        G_CALLBACK(inc_angle),&object );

  g_signal_connect(angle_dec, "clicked",
        G_CALLBACK(dec_angle),&object );

  g_signal_connect(proportion_inc, "clicked",
        G_CALLBACK(inc_proportion),&object );

  g_signal_connect(proportion_dec, "clicked",
        G_CALLBACK(dec_proportion),&object );


  gtk_widget_show_all(window);
  gtk_main();
  return 0;
}
