
// Example sketch which shows how to display some patterns
// on a 64x32 LED matrix
//
#define R1 25
#define G1 26
#define B1 27
#define R2 14
#define G2 12
#define B2 13
#define A 23
#define B 19
#define C 5
#define D 17
#define E 21 // assign to any available pin if using two panels or 64x64 panels with 1/32 scan
#define CLK 16
#define LAT 4
#define OE 15

#define NUM_PIECE_TYPES 5
#define BLOCK_SIZE 4

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#define PANEL_RES_X 64     // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 64     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 2    // Total number of panels chained one to another
 
//MatrixPanel_I2S_DMA dma_display;
MatrixPanel_I2S_DMA *dma_display = nullptr;

  // Module configuration
  HUB75_I2S_CFG::i2s_pins _pins={R1, G1, B1, R2, G2, B2, A, B, C, D, E, LAT, OE, CLK};
  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,   // module width
    PANEL_RES_Y,   // module height
    PANEL_CHAIN,    // Chain length
    _pins // pin mapping
  );

uint16_t myBlue = dma_display->color565(0, 0, 255);
uint16_t myCyan = dma_display->color565(0, 255, 255);
uint16_t myYellow = dma_display->color565(255, 255, 0);
uint16_t myRed = dma_display->color565(255, 0, 0);
uint16_t myPurple = dma_display->color565(160, 32, 240);
uint16_t myOrange = dma_display->color565(255, 165, 0);
uint16_t myGreen = dma_display->color565(0, 255, 0);
uint16_t RGB[]={myOrange,myBlue,myRed,myGreen,myPurple,myYellow,myCyan};//LJZSTOI

hw_timer_t * timer = NULL;
const int width =10;
const int height =20;
int piece_origin_x=(width/2-4/2);
int piece_origin_y=0;
int pieceIndex=0;
int* pieceIndexptr=&pieceIndex;
int draw_origin_x=12;
int draw_origin_y=47;
int* origin_y_ptr=&draw_origin_y;
int* origin_x_ptr=&draw_origin_x;
int currentRotation=0;
int* currentRotation_ptr=&currentRotation;
int score = 0;
int* score_ptr=&score;
int level = 1;
int* level_ptr=&level;
int lines = 0;
int* lines_ptr=&lines;

  int piece_L[4][4] = {
    {1, width+1, width*2+1, width*2+2},
    {width+2, width*2, width*2+1, width*2+2},
    {1, 2, width+2, width*2+2},
    {width, width+1, width+2, width*2}
  };
  
  int piece_J[4][4] = {
    {2, width+2, width*2+1, width*2+2},
    {width, width*2, width*2+1, width*2+2},
    {1, width+1, width*2+1, 2},
    {0, 1, 2, width+2}
  };

  int piece_Z[4][4] = {
    {0,width,width+1,width*2+1},
    {width+1, width+2,width*2,width*2+1},
    {0,width,width+1,width*2+1},
    {width+1, width+2,width*2,width*2+1}
  };

  int piece_S[4][4] = {
    {2,width+1,width+2,width*2+1},
    {width, width+1,width*2+1,width*2+2},
    {2,width+1,width+2,width*2+1},
    {width, width+1,width*2+1,width*2+2}
  };

  int piece_T[4][4] = {
    {1,width,width+1,width+2},//1,10,11,12
    {1,width+1,width+2,width*2+1},//1 11 12 21
    {width,width+1,width+2,width*2+1},// 10 11 12 21
    {1,width,width+1,width*2+1} // 1 10 11 21
  };

  int piece_O[4][4] = {
    {1,2,width+1,width+2},
    {1,2,width+1,width+2},
    {1,2,width+1,width+2},
    {1,2,width+1,width+2},
  };

  int piece_I[4][4] = {
    {width,width+1,width+2,width+3},
    {1,width+1,width*2+1,width*3+1},
    {width,width+1,width+2,width+3},
    {1,width+1,width*2+1,width*3+1}
  };

void drawText(){
  
  int text_size=3;
  dma_display->setTextSize(text_size);     // size 1 == 8 pixels high
  dma_display->setTextWrap(false); // Don't wrap at end of line - will do ourselves

  dma_display->setCursor(PANEL_RES_X-((18*6)/2), PANEL_RES_Y/2-(7*text_size)/2);
  uint8_t w = 0;
  const char *str = "TETRIS";
  for (w=0; w<strlen(str); w++) {
    dma_display->setTextColor(RGB[w]);
    dma_display->print(str[w]);
  }

};

void drawBoard(int Board[width][height+1]){
  for (int i=0; i<width; i++) {
    for (int j=0; j<height+1; j++) {
        if(Board[i][j]<7){
                    dma_display->fillRect(*origin_y_ptr+j*BLOCK_SIZE,*origin_x_ptr+i*BLOCK_SIZE,BLOCK_SIZE,BLOCK_SIZE,RGB[Board[i][j]]);
        }
    }
  }
};

void refreshBoard(int Board[width][height+1]){
  for (int i=0; i<width; i++) {
    for (int j=0; j<height+1; j++) {
        if(Board[i][j]==7){
                    dma_display->fillRect(*origin_y_ptr+j*BLOCK_SIZE,*origin_x_ptr+i*BLOCK_SIZE,BLOCK_SIZE,BLOCK_SIZE,0);
        }
    }
  }
};

void drawPiece(int piece[][4]){
    for (int i=0; i<4; i++) {
      dma_display->fillRect(*origin_y_ptr+piece[*currentRotation_ptr][i]/width*BLOCK_SIZE,*origin_x_ptr+(piece[*currentRotation_ptr][i]%width)*BLOCK_SIZE,BLOCK_SIZE,BLOCK_SIZE,RGB[pieceIndex]);
  }
};

void erasePiece(int piece[][4]){
    for (int i=0; i<4; i++) {
      dma_display->fillRect(*origin_y_ptr+piece[*currentRotation_ptr][i]/width*BLOCK_SIZE,*origin_x_ptr+(piece[*currentRotation_ptr][i]%width)*BLOCK_SIZE,BLOCK_SIZE,BLOCK_SIZE,0);
  }
};

void generatePiece(int Board[width][height+1],int piece[][4],int currentRotation,int pieces[4]){
  *currentRotation_ptr=0;
  if(pieces[0]<=1){
    *pieceIndexptr=0;
    for(int i=0;i<4;i++){
      for(int j=0;j<4;j++){
      piece[i][j]=piece_L[i][j]+width*piece_origin_y+piece_origin_x;
      }
    }
  }
  if(pieces[0]>1 && pieces[0]<=2){
    *pieceIndexptr=1;
    for(int i=0;i<4;i++){
      for(int j=0;j<4;j++){
      piece[i][j]=piece_J[i][j]+width*piece_origin_y+piece_origin_x;
      }
    }
  }
  if(pieces[0]>2 && pieces[0]<=3){
    *pieceIndexptr=2;
    for(int i=0;i<4;i++){
      for(int j=0;j<4;j++){
      piece[i][j]=piece_Z[i][j]+width*piece_origin_y+piece_origin_x;
      }
    }
  }
    if(pieces[0]>3 && pieces[0]<=4){
    *pieceIndexptr=3;
    for(int i=0;i<4;i++){
      for(int j=0;j<4;j++){
      piece[i][j]=piece_S[i][j]+width*piece_origin_y+piece_origin_x;
      }
    }
  }
    if(pieces[0]>4 && pieces[0]<=5){
    *pieceIndexptr=4;
    for(int i=0;i<4;i++){
      for(int j=0;j<4;j++){
      piece[i][j]=piece_T[i][j]+width*piece_origin_y+piece_origin_x;
      }
    }
  }
    if(pieces[0]>5 && pieces[0]<=6){
    *pieceIndexptr=5;
    for(int i=0;i<4;i++){
      for(int j=0;j<4;j++){
      piece[i][j]=piece_O[i][j]+width*piece_origin_y+piece_origin_x;
      }
    }
  }
    if(pieces[0]>6 && pieces[0]<=7){
    *pieceIndexptr=6;
    for(int i=0;i<4;i++){
      for(int j=0;j<4;j++){
      piece[i][j]=piece_I[i][j]+width*piece_origin_y+piece_origin_x;
      }
    }
  }
  for(int i=1;i<4;i++){
    pieces[i-1]=pieces[i];
  }
  pieces[3]=random(0,8);

  for(int i=0;i<4;i++){
    for(int j=0;j<4;j++){
      if(Board[(piece[currentRotation][i])%width][(piece[currentRotation][i])/width]<7){
        RestartGame(Board,piece,pieces);
      }
    }
    }
};

void lockPiece(int Board[width][height+1],int piece[][4],int colorIndex,int currentRotation,int pieces[4]){
  for(int i=0;i<4;i++){
    Board[(piece[currentRotation][i])%width][(piece[currentRotation][i])/width]=colorIndex;
  }
  checkLines(Board);
  dma_display->clearScreen();
  drawBoard(Board);
  generatePiece(Board,piece,currentRotation,pieces);
  drawPiece(piece);
};

void moveDown(int Board[width][height+1],int piece[][4],int currentRotation, int direction,int colorIndex,int pieces[4]){
  if(!checkVerticalCollision(Board,piece,currentRotation,direction)){
    erasePiece(piece);
    if(direction==1){
     for(int i=0;i<4;i++){
       for(int j=0;j<4;j++){
         piece[i][j]=piece[i][j]+width;
       }
     }
    }
    drawPiece(piece);
  }else{
    lockPiece(Board,piece,colorIndex,currentRotation,pieces);
  }
};

void moveHorizontaly(int Board[width][height+1],int piece[][4],int currentRotation, int direction){
  if(!checkHorizontalCollision(Board,piece,currentRotation,direction)){
    erasePiece(piece);
    if(direction==1){
     for(int i=0;i<4;i++){
       for(int j=0;j<4;j++){
         piece[i][j]=piece[i][j]+1;
       }
     }
    }else if (direction==-1){
     for(int i=0;i<4;i++){
       for(int j=0;j<4;j++){
         piece[i][j]=piece[i][j]-1;
       }
     }
    }
    drawPiece(piece);
  }
};

bool checkHorizontalCollision(int Board[width][height+1],int piece[][4],int currentRotation,int direction){
    for(int i =0;i<4;i++){
  int x1=(piece[currentRotation][i])%(width);
  int x2=(piece[currentRotation][i]+direction)%(width);
  int y1=(piece[currentRotation][i])/(width);
  int y2=(piece[currentRotation][i]+direction)/(width);
  if(y2!=y1){//check for collision with border
    return true;
  }
  if(Board[x2][y2]<7){//check for collision with locked pieces
    return true;
  }
  }
  return false;
};

bool checkVerticalCollision(int Board[width][height+1],int piece[][4],int currentRotation,int direction){
  for(int i=0;i<4;i++){
    if(Board[(piece[currentRotation][i])%width][(piece[currentRotation][i]+width)/width]<7){
      return true;
    }
  }
  return false;
};

void rotatePiece(int Board[width][height+1],int piece[][4]){
  bool collision=false;
  int temp= *currentRotation_ptr<3?*currentRotation_ptr+1:0;
  for(int i=0;i<4;i++){
    if((Board[piece[temp][i]%(width)][piece[temp][i]/(width)]<7)){
        collision = true;
    }
  }
  if(!collision){
    erasePiece(piece);
    *currentRotation_ptr<3?*currentRotation_ptr=*currentRotation_ptr+1:*currentRotation_ptr=0;
    drawPiece(piece);
  }
};

void generatePieces(int pieces[4]){
  for(int i=0;i<4;i++){
     pieces[i]=random(0,8);
  }
};

void RestartGame(int Board[width][height+1],int piece[][4],int pieces[4]){
  dma_display->clearScreen();
  for(int i=0;i<width;i++){
    for(int j=0;j<height;j++){
      Board[i][j]=7;
    }
  }
  for(int i=0;i<width;i++){
    Board[i][height]=1;
  }
  generatePieces(pieces);
  generatePiece(Board,piece,*currentRotation_ptr,pieces);
  drawBoard(Board);
  drawPiece(piece);

};

void checkLines(int Board[width][height+1]){
  int linesCounter=0;
    for (int j=0; j<height; j++) {
      int counter=0;
      for (int i=0; i<width; i++){
        if(Board[i][j]<7){
          counter++;
        }
        if(counter==width){
          *lines_ptr=*lines_ptr+1;
          *level_ptr=*lines_ptr/10+1;
          linesCounter++;
          for(int k=j; k>0; k--){
            for(int i=0;i<width;i++){
              Board[i][k]=Board[i][k-1];
            }
          }
          if(j==0){
            for(int i=0;i<width;i++){
              Board[i][j]=7;
            }
          }
        }
      }
    }
    if(linesCounter==1){
      *score_ptr=*score_ptr+(*level_ptr*100);
      }
      else if(linesCounter==2){
      *score_ptr=*score_ptr+(*level_ptr*300);
      }
      else if(linesCounter==3){
      *score_ptr=*score_ptr+(*level_ptr*500);
      }
      else if(linesCounter==4){
      *score_ptr=*score_ptr+(*level_ptr*800);
      }
};

int Pieces[4]={0};
int Board[width][height+1]={0};
int ActivePiece[4][4]={0};
String str;

void IRAM_ATTR timer_isr() {
    moveDown(Board,ActivePiece,currentRotation,1,pieceIndex,Pieces);
}

void setup() {
  Serial.begin(230400);
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(80); //0-255
  dma_display->clearScreen();
  drawText();
  delay(3000);
  dma_display->clearScreen();
  RestartGame(Board,ActivePiece,Pieces);
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &timer_isr, true);
  timerAlarmWrite(timer, 1000000, true);
  timerAlarmEnable(timer);
}

void loop() {
  if(Serial.available()){
    char temp=Serial.read();
    if(temp=='q'){
      Serial.print("q");
      moveHorizontaly(Board,ActivePiece,currentRotation,-1);
      str="";
    } else if(temp=='w'){
      Serial.print("w");
      rotatePiece(Board,ActivePiece);
      str="";
    } else if(temp=='e'){
      Serial.print("e");
      moveHorizontaly(Board,ActivePiece,currentRotation,1);
      str="";
    } else if(temp=='r'){
      Serial.print("r");
      moveDown(Board,ActivePiece,currentRotation,1,pieceIndex,Pieces);
      *score_ptr=*score_ptr+1;
      str="";      
    } else if(temp=='t'){
      Serial.print("t");
      moveDown(Board,ActivePiece,currentRotation,1,pieceIndex,Pieces);
      str="";      
    }else if(temp=='\n'){
    }else{
      str +=temp;
    }
  }
}
