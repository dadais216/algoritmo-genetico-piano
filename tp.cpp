
#include <cstdlib>
#include <ctime>
#include <SFML/Audio.hpp>
#include <stdio.h>
#include <cstring>
#include <unistd.h>
#include <algorithm>

const int duracion=10;
const int tempo=4;
const int tamanoPoblacion=100;
typedef int cancion[duracion*tempo];
typedef cancion poblaciont[tamanoPoblacion];

poblaciont poblacion1;
poblaciont poblacion2;
poblaciont* poblacionAct;
poblaciont* poblacionSwap;
//hay un swap de memorias porque cada iteracion vuelca su informacion en la proxima,
//en vez de tener una cadena infinita tengo 2 que voy rotando

int iteracion=0;

struct rank{
  int ind;
  int apt;
};
rank ranking[tamanoPoblacion];

int random(int min,int max){ //buscar un generador mas piola a ver si cambia los resultados
  return rand()%(max+1-min)+min;
}

sf::SoundBuffer buffers[68];
sf::Sound sounds[68];
void play(int* c){
  for(int n=0;n<duracion*tempo;n++){
    int nota=c[n];
    printf("%2d  ",nota);
    sounds[nota].play();
    usleep((1000/tempo)*1000);
  }
  //usleep((1000/tempo)*7*1000);
}

int absDiff(int a,int b){
  int diff=a-b;
  return diff>0?diff:-diff;
}

int aptitud(int* c){
  int apt=0;

  int loop=8;
  for(int i=0;i<duracion*tempo-loop;i++){
    apt-=absDiff(c[i],c[i+loop])*8;
  }
  for(int i=0;i<duracion*tempo-loop*2;i++){
    int diff=absDiff(c[i],c[i+loop*2]);
    apt-=diff>16?0:(16-diff)*1;
  }


  int scaleStep=0;
  int scaleLimit;
  for(int i=0;i<duracion*tempo-1;i++){
    int diff=c[i+1]-c[i];

    int jmp=2;

    //escala mayor
    //if(scaleStep==3||scaleStep==6) jmp=4;
    //scaleLimit=7;

    //escala menor
    if(scaleStep==1||scaleStep==5) jmp=4;
    scaleLimit=7;

    //escala lydian aug
    //if(scaleStep==3) jmp=0;
    //if(scaleStep==6||scaleStep==7) jmp=3;
    //scaleLimit=8;

    //escala cromatica
    //if(scaleStep%2==0) jmp=1;
    //if(scaleStep>=6) jmp*=-1;
    //scaleLimit=12;

    scaleStep++;
    scaleStep=scaleStep%scaleLimit;

    apt-=absDiff(diff,2*jmp)*1;
  }
  return apt;
}

void seleccion(){
  for(int i=0;i<tamanoPoblacion;i++){
    ranking[i].ind=i;
    ranking[i].apt=aptitud((*poblacionAct)[i]);
  }
  std::sort(&ranking[0],&ranking[tamanoPoblacion],[](rank& a,rank& b)->bool{
                                                      return a.apt>b.apt;
                                                });
  //printf("song %d apt %d\n",ranking[99].ind,ranking[99].apt);
  //play((*poblacionAct)[ranking[99].ind]);
  if(iteracion>0&&iteracion%50==0){
    printf("~~~%d~~~\n",iteracion);
    printf("song %d apt %d\n",ranking[0].ind,ranking[0].apt);
    play((*poblacionAct)[ranking[0].ind]);
  }
}

const int tamanoSeleccion=50;
const int tamanoCorteMaximo=20;

int seleccionarParaPareja(int yaCasados){
  int ind=random(0,tamanoSeleccion-yaCasados-1);
  int ret=ranking[ind].ind;

  //los doy vuelta porque necesito seguir teniendo el eliminado si hay mas de una ronda de emparejado
  rank swapTemp=ranking[ind];
  ranking[ind]=ranking[tamanoSeleccion-yaCasados];
  ranking[tamanoSeleccion-yaCasados]=swapTemp;

  return ret;
}
void cruzamiento(){
  //mezclo todos asegurando que se tenga al menos un hijo cada uno, para no perder informacion

  int yaCasados=0;
  for(int i=0;i<tamanoPoblacion;i+=2){
    if(yaCasados==tamanoSeleccion){
      yaCasados=0;
    }
    int papa=seleccionarParaPareja(yaCasados);yaCasados++;
    int mama=seleccionarParaPareja(yaCasados);yaCasados++;


    bool endReached=false;
    int genActual=0;
    int corteAnterior=0;
    for(int c=0;!endReached;c++){
      int puntoDeCorte=random(corteAnterior+1,corteAnterior+tamanoCorteMaximo);//tipo poisson
      if(puntoDeCorte>=duracion*tempo){
        puntoDeCorte=duracion*tempo;
        endReached=true;
      }
      corteAnterior=puntoDeCorte;
      
      if(c%2==0){
        for(;genActual<puntoDeCorte;genActual++){
          (*poblacionSwap)[i]  [genActual]=(*poblacionAct)[papa][genActual];
          (*poblacionSwap)[i+1][genActual]=(*poblacionAct)[mama][genActual];
        }
      }else{
        for(;genActual<puntoDeCorte;genActual++){
          (*poblacionSwap)[i]  [genActual]=(*poblacionAct)[mama][genActual];
          (*poblacionSwap)[i+1][genActual]=(*poblacionAct)[papa][genActual];
        }
      }
    }
  }
}

void mutacion(){
  for(int i=0;i<tamanoPoblacion;i++){
    int cantMutacion=random(1,4);
    for(int m=0;m<cantMutacion;m++){
      int* val=&(*poblacionSwap)[i][random(0,duracion*tempo)];
      int variacion=random(1,4)-2;
      int valF=*val+variacion;
      if(valF<0) valF=0;
      if(valF>63) valF=63;
      *val=valF;
    }
  }
}

int main(){
  srand(time(0));
  setbuf(stdout,NULL);

  char filename[100];
  strcpy(filename,"jobro__piano-ff/jobro__piano-ff-0xx.wav");

  for(int i=1;i<=68;i++){
    filename[33]=i/10+'0';
    filename[34]=i%10+'0';
    if (!buffers[i-1].loadFromFile(filename))
      return -1;
    sounds[i-1].setBuffer(buffers[i-1]);
  }

  for(int i=0;i<tamanoPoblacion;i++){
    for(int n=0;n<duracion*tempo;n++){
      poblacion1[i][n]=random(0,63);
    }
  }

  while(true){
    if(iteracion%2==0){
      poblacionAct=&poblacion1;
      poblacionSwap=&poblacion2;
    }else{
      poblacionAct=&poblacion2;
      poblacionSwap=&poblacion1;
    }
    seleccion();
    cruzamiento();
    mutacion();

    iteracion++;
  }


  return 0;
}


/*

  sounds[ran].play();
  usleep(125*1000);

  sounds[ran+4].play();
  usleep(125*1000);

  sounds[ran].stop();
  sounds[ran+4].stop();

*/





