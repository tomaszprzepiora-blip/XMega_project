enum GlcdColor {_clClear, _clDraw, _clInvert};

typedef struct Screen TScreen;

typedef struct  Label {
  TScreen*  OwnerScreen;
  char          Order;
  unsigned int  Left;
  unsigned int  Top;
  unsigned int  Width;
  unsigned int  Height;
  char          *Caption;
  const far char    *FontName;
  unsigned int  Font_Color;
  char          Visible;
  char          Active;
  void          (*OnUpPtr)();
  void          (*OnDownPtr)();
  void          (*OnClickPtr)();
  void          (*OnPressPtr)();
} TLabel;

typedef struct  Image {
  TScreen*  OwnerScreen;
  char          Order;
  unsigned int  Left;
  unsigned int  Top;
  unsigned int  Width;
  unsigned int  Height;
  const far char    *Picture_Name;
  char          Visible;
  char          Active;
  char          Picture_Type;
  char          Picture_Ratio;
  void          (*OnUpPtr)();
  void          (*OnDownPtr)();
  void          (*OnClickPtr)();
  void          (*OnPressPtr)();
} TImage;

typedef struct  Circle {
  TScreen*  OwnerScreen;
  char          Order;
  unsigned int  Left;
  unsigned int  Top;
  unsigned int  Radius;
  char          Pen_Width;
  unsigned int  Pen_Color;
  char          Visible;
  char          Active;
  char          Transparent;
  char          Gradient;
  char          Gradient_Orientation;
  unsigned int  Gradient_Start_Color;
  unsigned int  Gradient_End_Color;
  unsigned int  Color;
  char          PressColEnabled;
  unsigned int  Press_Color;
  void          (*OnUpPtr)();
  void          (*OnDownPtr)();
  void          (*OnClickPtr)();
  void          (*OnPressPtr)();
} TCircle;

struct Screen {
  unsigned int           Color;
  unsigned int           Width;
  unsigned int           Height;
  unsigned short         ObjectsCount;
  unsigned int           LabelsCount;
  TLabel                 * const code far *Labels;
  unsigned int           ImagesCount;
  TImage                 * const code far *Images;
  unsigned int           CirclesCount;
  TCircle                * const code far *Circles;
};

extern   TScreen                Screen1;
extern   TImage               Image1;
extern   TCircle                Circle1;
extern   TCircle                Circle2;
extern   TCircle                Circle3;
extern   TCircle                Circle4;
extern   TCircle                Circle5;
extern   TCircle                Circle6;
extern   TCircle                Circle7;
extern   TCircle                Circle8;
extern   TCircle                Circle9;
extern   TCircle                Circle10;
extern   TCircle                Circle11;
extern   TCircle                Circle12;
extern   TCircle                Circle13;
extern   TCircle                Circle14;
extern   TCircle                Circle15;
extern   TLabel                 Label1;
extern   TLabel                 Label2;
extern   TCircle                Circle16;
extern   TCircle                Circle17;
extern   TCircle                Circle18;
extern   TCircle                Circle20;
extern   TLabel                 * const code far Screen1_Labels[2];
extern   TImage                 * const code far Screen1_Images[1];
extern   TCircle                * const code far Screen1_Circles[19];



/////////////////////////
// Events Code Declarations
/////////////////////////

/////////////////////////////////
// Caption variables Declarations
extern char Label1_Caption[];
extern char Label2_Caption[];
/////////////////////////////////

void DrawScreen(TScreen *aScreen);
void DrawLabel(TLabel *ALabel);
void DrawImage(TImage *AImage);
void DrawCircle(TCircle *ACircle);
void Check_TP();
void Start_TP();
