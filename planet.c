/* planet.c */
/* planet generating program */
/* Copyright 1988--present Torben AE. Mogensen */

char version[] = "August 14 2024";

/* The program generates planet maps based on recursive spatial subdivision */
/* of a tetrahedron containing the globe. The output is a colour BMP bitmap. */
/* with options for PPM or B/W bitmaps */

/* The colours may optionally be modified according to latitude to move the */
/* icecaps lower closer to the poles, with a corresponding change in land colours. */

/* The Mercator map at magnification 1 is scaled to fit the Width */
/* it uses the full height (it could extend infinitely) */
/* The orthographic projections are scaled so the full view would use the */
/* full Height. Areas outside the globe are coloured black. */
/* Stereographic and gnomonic projections use the same scale as orthographic */
/* in the center of the picture, but distorts scale away from the center. */

/* It is assumed that pixels are square */
/* I have included procedures to print the maps as bmp (Windows) or */
/* ppm(portable pixel map) bitmaps  on standard output or specified files. */

/* I have tried to avoid using machine specific features, so it should */
/* be easy to port the program to any machine. Beware, though that due */
/* to different precision on different machines, the same seed numbers */
/* can yield very different planets. */

/* The primitive user interface is primarily a result of portability concerns */


#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifdef THINK_C
#define macintosh 1
#endif

#ifdef macintosh
#include <console.h>
#include <unix.h>
#endif

#ifdef WIN32
#include <io.h>
#undef min
#undef max
#define O_BINARY 0x8000
#elif _WIN32
#include <io.h>
#undef min
#undef max
#define O_BINARY 0x8000
#endif

int BLACK = 0;
int WHITE = 1;
int BACK = 2;
int GRID = 3;
int OUTLINE1 = 4;
int OUTLINE2 = 5;
int LOWEST = 6;
int SEA = 7;
int LAND = 8;
int HIGHEST = 9;

int nonLinear = 0;

char view;

int nocols = 65536;

int rtable[65536], gtable[65536], btable[65536];

/* Supported output file types:
    BMP - Windows Bit MaPs
    PPM - Portable Pix Maps
    XPM - X-windows Pix Maps
 */

double log_2(x)
double x;
{ return(log(x)/log(2.0)); }

typedef enum ftype
    {
        bmp,
        ppm,
        xpm,
    heightfield
    }
    ftype;

ftype file_type = bmp;

char* file_ext(ftype file_type)
{
  switch (file_type)
  {
    case bmp:
      return (".bmp");
    case ppm:
      return (".ppm");
    case xpm:
      return (".xpm");
    case heightfield:
      return (".heightfield");
    default:
      return ("");
  }
}

/* Character table for XPM output */

char letters[64] = {
        '@','$','.',',',':',';','-','+','=','#','*','&','A','B','C','D',
        'E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T',
        'U','V','W','X','Y','Z','a','b','c','d','e','f','g','h','i','j',
        'k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};


/* T = tundra, G = grasslands, B = Taiga / boreal forest, D = desert,
   S = savanna, F = temperate forest, R = temperate rainforest,
   W = Xeric shrubland and dry forest, E = tropical dry forest,
   O = tropical rainforest, I = icecap */

/* Whittaker diagram */
char biomes[45][45] = {
		       "IIITTTTTGGGGGGGGDDDDDDDDDDDDDDDDDDDDDDDDDDDDD",
		       "IIITTTTTGGGGGGGGDDDDGGDSDDSDDDDDDDDDDDDDDDDDD",
		       "IITTTTTTTTTBGGGGGGGGGGGSSSSSSDDDDDDDDDDDDDDDD",
		       "IITTTTTTTTBBBBBBGGGGGGGSSSSSSSSSWWWWWWWDDDDDD",
		       "IITTTTTTTTBBBBBBGGGGGGGSSSSSSSSSSWWWWWWWWWWDD",
		       "IIITTTTTTTBBBBBBFGGGGGGSSSSSSSSSSSWWWWWWWWWWW",
		       "IIIITTTTTTBBBBBBFFGGGGGSSSSSSSSSSSWWWWWWWWWWW",
		       "IIIIITTTTTBBBBBBFFFFGGGSSSSSSSSSSSWWWWWWWWWWW",
		       "IIIIITTTTTBBBBBBBFFFFGGGSSSSSSSSSSSWWWWWWWWWW",
		       "IIIIIITTTTBBBBBBBFFFFFFGGGSSSSSSSSWWWWWWWWWWW",
		       "IIIIIIITTTBBBBBBBFFFFFFFFGGGSSSSSSWWWWWWWWWWW",
		       "IIIIIIIITTBBBBBBBFFFFFFFFFFGGSSSSSWWWWWWWWWWW",
		       "IIIIIIIIITBBBBBBBFFFFFFFFFFFFFSSSSWWWWWWWWWWW",
		       "IIIIIIIIIITBBBBBBFFFFFFFFFFFFFFFSSEEEWWWWWWWW",
		       "IIIIIIIIIITBBBBBBFFFFFFFFFFFFFFFFFFEEEEEEWWWW",
		       "IIIIIIIIIIIBBBBBBFFFFFFFFFFFFFFFFFFEEEEEEEEWW",
		       "IIIIIIIIIIIBBBBBBRFFFFFFFFFFFFFFFFFEEEEEEEEEE",
		       "IIIIIIIIIIIIBBBBBBRFFFFFFFFFFFFFFFFEEEEEEEEEE",
		       "IIIIIIIIIIIIIBBBBBRRRFFFFFFFFFFFFFFEEEEEEEEEE",
		       "IIIIIIIIIIIIIIIBBBRRRRRFFFFFFFFFFFFEEEEEEEEEE",
		       "IIIIIIIIIIIIIIIIIBRRRRRRRFFFFFFFFFFEEEEEEEEEE",
		       "IIIIIIIIIIIIIIIIIRRRRRRRRRRFFFFFFFFEEEEEEEEEE",
		       "IIIIIIIIIIIIIIIIIIRRRRRRRRRRRRFFFFFEEEEEEEEEE",
		       "IIIIIIIIIIIIIIIIIIIRRRRRRRRRRRRRFRREEEEEEEEEE",
		       "IIIIIIIIIIIIIIIIIIIIIRRRRRRRRRRRRRRRREEEEEEEE",
		       "IIIIIIIIIIIIIIIIIIIIIIIRRRRRRRRRRRRRROOEEEEEE",
		       "IIIIIIIIIIIIIIIIIIIIIIIIRRRRRRRRRRRROOOOOEEEE",
		       "IIIIIIIIIIIIIIIIIIIIIIIIIIRRRRRRRRRROOOOOOEEE",
		       "IIIIIIIIIIIIIIIIIIIIIIIIIIIRRRRRRRRROOOOOOOEE",
		       "IIIIIIIIIIIIIIIIIIIIIIIIIIIIRRRRRRRROOOOOOOEE",
		       "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIRRRRRRROOOOOOOOE",
		       "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIRRRRROOOOOOOOOO",
		       "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIRROOOOOOOOOOO",
		       "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIROOOOOOOOOOO",
		       "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIROOOOOOOOOOO",
		       "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIOOOOOOOOOOO",
		       "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIOOOOOOOOOO",
		       "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIOOOOOOOOO",
		       "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIOOOOOOOOO",
		       "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIOOOOOOOO",
		       "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIOOOOOOOO",
		       "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIOOOOOOOO",
		       "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIOOOOOOOO",
		       "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIOOOOOOO",
		       "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIOOOOOOO"
};
      
  
#define PI 3.14159265358979323846
#define DEG2RAD 0.01745329251994329576923 /* pi/180 */

/* these three values can be changed to change world characteristica */

double M  = -.02;   /* initial altitude (slightly below sea level) */
double dd1 = 0.45;  /* weight for altitude difference */
double POWA = 1.0; /* power for altitude difference */
double dd2 = 0.035; /* weight for distance */
double POW = 0.47;  /* power for distance function */

int Depth; /* depth of subdivisions */
double r1,r2,r3,r4; /* seeds */
double longi,lat,scale;
double vgrid, hgrid;

int latic = 0; /* flag for latitude based colour */

int Width = 800, Height = 600; /* default map size */

unsigned short **col;  /* colour array */
int **heights;         /* heightfield array */
double **xxx, **yyy, **zzz; /* x,y,z arrays  (used for gridlines */
int cl0[1000][1000]; /* match map */
int MatchWidth = 48, MatchHeight = 24;

int do_outline = 0;  /* if 1, draw coastal outline */
int do_bw = 0;       /* if 1, reduce map to black outline on white */
int contourLines = 0; /* if >0, # of contour lines */
int coastContourLines = 0; /* if >0, # of coastal contour lines */
int *outx, *outy;

int doshade = 0;
int shade;
unsigned short **shades; /* shade array */
double shade_angle = 150.0; /* angle of "light" on bumpmap */
double shade_angle2 = 20.0; /* with daylight shading, these two are
                               longitude/latitude */

double cla, sla, clo, slo, rseed;

int temperature = 0; /* if 1, show temperatures based on latitude
			and altitude*/
double tempMin = 1000.0, tempMax = -1000.0;

int rainfall = 0; /* if 1, calculate rainfall based on latitude
			and temperature */
double rainMin = 1000.0, rainMax = -1000.0;

double rainShadow = 0.0; /* approximate rain shadow */

int makeBiomes = 0; /* if 1, make biome map */

int matchMap = 0;
double matchSize = 0.1;

typedef struct Vertex
{
  double h; /* altitude */
  double s; /* seed */
  double x,y,z; /* coordinates */
  double shadow; /* approximate rain shadow */
} vertex;

/* distance squared between vertices */
double dist2(vertex a, vertex b)
{
  double abx, aby, abz;
  abx = a.x-b.x; aby = a.y-b.y; abz = a.z-b.z;
  return abx*abx+aby*aby+abz*abz;
}

/* For the vertices of the tetrahedron */
vertex tetra[4];

double rotate1 = 0.0, rotate2 = 0.0;
double cR1, sR1, cR2, sR2;

char cmdLine[1000]; /* command line info */

int min(x,y)
int x,y;
{ return(x<y ? x : y); }

int max(x,y)
int x,y;
{ return(x<y ? y : x); }

double fmin(x,y)
double x,y;
{ return(x<y ? x : y); }

double fmax(x,y)
double x,y;
{ return(x<y ? y : x); }

int main(ac,av)
int ac;
char **av;
{
  void printppm(), printppmBW(), printbmp(), printbmpBW(),
    printxpm(), printxpmBW(), printheights(), print_error(), reportError(char c);
  void mercator(), peter(), squarep(), mollweide(), sinusoid(), stereo(),
    orthographic(), gnomonic(), icosahedral(), azimuth(), conical();
  int i;
  double rand2(),  planet1();
  void readcolors();
  void readmap(), makeoutline(), smoothshades();
  FILE *outfile, *colfile = NULL;
  char filename[256] = "planet-map";
  char colorsname[256] = "Olsson.col";
  char biocolorsname[256] = "default.bio";
  int do_file = 0, tmp = 0;
  double tx, ty, tz;

  /* initialize vertices to slightly irregular tetrahedron */
  tetra[0].x = -sqrt(3.0)-0.20;
  tetra[0].y = -sqrt(3.0)-0.22;
  tetra[0].z = -sqrt(3.0)-0.23;

  tetra[1].x = -sqrt(3.0)-0.19;
  tetra[1].y = sqrt(3.0)+0.18;
  tetra[1].z = sqrt(3.0)+0.17;

  tetra[2].x = sqrt(3.0)+0.21;
  tetra[2].y = -sqrt(3.0)-0.24;
  tetra[2].z = sqrt(3.0)+0.15;

  tetra[3].x = sqrt(3.0)+0.24;
  tetra[3].y = sqrt(3.0)+0.22;
  tetra[3].z = -sqrt(3.0)-0.25;


#ifdef macintosh
  _ftype = 'TEXT';
  _fcreator ='ttxt';

  ac = ccommand (&av);
  do_file = 1;
#endif

  longi = 0.0;
  lat = 0.0;
  scale = 1.0;
  rseed = 0.123;
  view = 'm';
  vgrid = hgrid = 0.0;
  outfile = stdout;

#ifdef WIN32
  _setmode(fileno(outfile), O_BINARY);
#elif _WIN32
  _setmode(fileno(outfile), O_BINARY);
#endif

  strcpy(cmdLine,"");
  for (i = 0; i<ac; i++) {
    strcat(cmdLine,av[i]);
    strcat(cmdLine," ");
  }
  for (i = 1; i<ac; i++) {
    if (av[i][0] == '-') {
      switch (av[i][1]) {
        case 'V' : if (++i < ac &&  sscanf(av[i],"%lf",&dd2)) break;
	           reportError('V');
        case 'v' : if (++i < ac &&  sscanf(av[i],"%lf",&dd1)) break;
	           reportError('v');
        case 's' : if (++i < ac &&  sscanf(av[i],"%lf",&rseed)) break;
		   reportError('s');
        case 'w' : if (++i < ac &&  sscanf(av[i],"%d",&Width)) break;
		   reportError('w');
        case 'h' : if (++i < ac && sscanf(av[i],"%d",&Height)) break;
		   reportError('h');
        case 'm' : if (++i < ac && sscanf(av[i],"%lf",&scale)) {
	             if (scale < 0.1) scale = 0.1;
                     break;
		   }
		   reportError('m');
        case 'o' : if (++i < ac && sscanf(av[i],"%255[^\n]",filename)) {
                     do_file = 1;
                     break;
		   }
		   reportError('o');
        case 'x' : file_type =xpm;
                   break;
        case 'R' : fprintf(stderr, "Version: %s\n", version);
                   break;
        case 'C' : if (++i < ac && sscanf(av[i],"%255[^\n]",colorsname)) break;
		   reportError('C');
        case 'Z' : if (++i < ac && sscanf(av[i],"%255[^\n]",biocolorsname)) break;
		   reportError('Z');
        case 'l' : if (++i < ac && sscanf(av[i],"%lf",&longi)) {
                     while (longi<-180) longi += 360;
                     while (longi>180) longi -= 360;
                     break;
	           }
		   reportError('l');
	case 'L' : if (++i < ac && sscanf(av[i],"%lf",&lat)) {
                     if (lat<-90) lat = -90;
                     if (lat>90) lat = 90;
                     break;
		   }
		   reportError('L');
        case 'g' : if (++i < ac && sscanf(av[i],"%lf",&vgrid)) break;
		   reportError('g');
        case 'G' : if (++i < ac && sscanf(av[i],"%lf",&hgrid)) break;
		   reportError('G');
        case 'c' : latic += 1;
                   break;
        case 'S' : dd1 /= 2.0; POWA = 0.75;
                   break;
        case 'n' : nonLinear = 1;
                   break;
        case 'O' : do_outline = 1;
                   do_bw = 1;
                   if (strlen(av[i])>2) {
                     sscanf(av[i],"-O%d",&tmp);
		     if (tmp < 0) coastContourLines = -tmp;
		     else contourLines = tmp;
		   }
                   break;
        case 'E' : do_outline = 1;
                   if (strlen(av[i])>2) {
                     sscanf(av[i],"-E%d",&tmp);
		     if (tmp < 0) coastContourLines = -tmp;
		     else contourLines = tmp;
		   }
                   break;
        case 'B' : doshade = 1; /* bump map */
                   break;
        case 'b' : doshade = 2; /* bump map on land only */
                   break;
        case 'd' : doshade = 3; /* daylight shading */
                   break;
        case 'P' : file_type = ppm;
                   break;
        case 'H' : file_type = heightfield;
                   break;
        case 'M' : if (i+1 < ac && sscanf(av[i+1],"%lf",&matchSize)) i++;
	           else matchSize = 0.1;
                   matchMap = 1;
    	           break;
        case 'a' : if (++i < ac && sscanf(av[i],"%lf",&shade_angle)) break;
		   reportError('a');
        case 'A' : if (++i < ac && sscanf(av[i],"%lf",&shade_angle2)) break;
		   reportError('A');
        case 'i' : if (++i < ac && sscanf(av[i],"%lf",&M)) break;
		   reportError('i');
        case 'T' : if (++i < ac && sscanf(av[i]," %lf",&rotate2))
		     if (++i < ac && sscanf(av[i]," %lf",&rotate1)) {
                       while (rotate1<-180) rotate1 += 360;
                       while (rotate1>180) rotate1 -= 360;
                       while (rotate2<-180) rotate2 += 360;
                       while (rotate2>180) rotate2 += 360;
                       break;
		     }
		   reportError('T');
        case 't' : temperature = 1; break;
        case 'r' : rainfall = 1; break;
        case 'z' : makeBiomes = 1; break;
        case 'p' : if (strlen(av[i])>2) view = av[i][2];
                   else if (++i < ac) view = av[i][0]; else view = ' ';
                   switch (view) {
                     case 'm' :
                     case 'p' :
                     case 'q' :
                     case 's' :
                     case 'o' :
                     case 'g' :
                     case 'a' :
                     case 'c' :
                     case 'M' :
                     case 'S' :
                     case 'i' : break;
                     case 'h' : file_type = heightfield; break;
                     default: fprintf(stderr,"Unknown projection: %s\n",av[i]);
                              print_error();
                   }
                   break;
        default: fprintf(stderr,"Unknown option: %s\n",av[i]);
                 print_error();
      }
    }
    else {
      fprintf(stderr,"Unknown option: %s\n\n",av[i]);
      print_error();
    }
  }

  readcolors(colfile, colorsname, biocolorsname);

#ifdef macintosh
    switch (file_type)
    {
      case bmp:
        _ftype = 'BMPf';
        break;
      case ppm:
        _ftype = 'PPGM';
        break;
      case xpm:
        _ftype = 'TEXT';
        break;
    }

    _fcreator ='GKON';
#endif

  if (do_file &&'\0' != filename[0]) {
    if (strchr (filename, '.') == 0)
      strcpy(&(filename[strlen(filename)]), file_ext(file_type));

    outfile = fopen(filename,"wb");

#ifdef macintosh
    _ftype = 'TEXT';
    _fcreator ='ttxt';
#endif

    if (outfile == NULL) {
      fprintf(stderr,
              "Could not open output file %s, error code = %d\n",
              filename, errno);
      exit(0);
    }
  }
  else {
    outfile = stdout;
#ifdef WIN32
    setmode(fileno(outfile), O_BINARY);
#elif _WIN32
    setmode(fileno(outfile), O_BINARY);
#endif
  }

  longi = longi*DEG2RAD;
  lat = lat*DEG2RAD;

  sla = sin(lat); cla = cos(lat);
  slo = sin(longi); clo = cos(longi);

  rotate1 = -rotate1*DEG2RAD;
  rotate2 = -rotate2*DEG2RAD;

  sR1 = sin(rotate1); cR1 = cos(rotate1);
  sR2 = sin(rotate2); cR2 = cos(rotate2);

  for (i=0; i<4; i++) { /* rotate around y axis */
    tx = tetra[i].x;
    ty = tetra[i].y;
    tz = tetra[i].z;
    tetra[i].x = cR2*tx + sR2*tz;
    tetra[i].y = ty ;
    tetra[i].z = -sR2*tx + cR2*tz;
  }

  for (i=0; i<4; i++) { /* rotate around x axis */
    tx = tetra[i].x;
    ty = tetra[i].y;
    tz = tetra[i].z;
    tetra[i].x = tx;
    tetra[i].y = cR1*ty - sR1*tz ;
    tetra[i].z = sR1*ty + cR1*tz;
    }

  if (matchMap) readmap();

  if (file_type == heightfield) {
    heights = (int**)calloc(Width,sizeof(int*));
    if (heights == 0) {
      fprintf(stderr, "Memory allocation failed.");
      exit(1);
    }
    for (i=0; i<Width; i++) {
      heights[i] = (int*)calloc(Height,sizeof(int));
      if (heights[i] == 0) {
        fprintf(stderr,
                "Memory allocation failed at %d out of %d heights\n",
                i+1,Width);
        exit(1);
      }
    }
  }

  col = (unsigned short**)calloc(Width,sizeof(unsigned short*));
  if (col == 0) {
    fprintf(stderr, "Memory allocation failed.");
    exit(1);
  }
  for (i=0; i<Width; i++) {
    col[i] = (unsigned short*)calloc(Height,sizeof(unsigned short));
    if (col[i] == 0) {
      fprintf(stderr,
              "Memory allocation failed at %d out of %d cols\n",
              i+1,Width);
      exit(1);
    }
  }

  if (doshade>0) {
    shades = (unsigned short**)calloc(Width,sizeof(unsigned short*));
    if (shades == 0) {
      fprintf(stderr, "Memory allocation failed.");
      exit(1);
    }
    for (i=0; i<Width; i++) {
      shades[i] = (unsigned short*)calloc(Height,sizeof(unsigned short));
      if (shades[i] == 0) {
        fprintf(stderr,
                "Memory allocation failed at %d out of %d shades\n",
                i,Width);
        exit(1);
      }
    }
  }

  if (vgrid != 0.0) {
    xxx = (double**)calloc(Width,sizeof(double*));
    if (xxx == 0) {
      fprintf(stderr, "Memory allocation failed xxx.");
      exit(1);
    }
    for (i=0; i<Width; i++) {
      xxx[i] = (double*)calloc(Height,sizeof(double));
      if (xxx[i] == 0) {
        fprintf(stderr,
                "Memory allocation failed at %d out of %d xxx\n",
                i+1,Width);
        exit(1);
      }
    }

    zzz = (double**)calloc(Width,sizeof(double*));
    if (zzz == 0) {
      fprintf(stderr, "Memory allocation failed zzz.");
      exit(1);
    }
    for (i=0; i<Width; i++) {
      zzz[i] = (double*)calloc(Height,sizeof(double));
      if (zzz[i] == 0) {
        fprintf(stderr,
                "Memory allocation failed at %d out of %d zzz\n",
                i+1,Width);
        exit(1);
      }
    }
  }

  if (hgrid != 0.0 || vgrid != 0.0) {
    yyy = (double**)calloc(Width,sizeof(double*));
    if (yyy == 0) {
      fprintf(stderr, "Memory allocation failed yyy.");
      exit(1);
    }
    for (i=0; i<Width; i++) {
      yyy[i] = (double*)calloc(Height,sizeof(double));
      if (yyy[i] == 0) {
        fprintf(stderr,
                "Memory allocation failed at %d out of %d yyy\n",
                i+1,Width);
        exit(1);
      }
    }
  }

  if (view == 'c' || view == 'm') {
    if (lat == 0) view = 'm';
        /* Conical approaches mercator when lat -> 0 */
    if (fabs(lat) >= PI - 0.000001) view = 's';
        /* Conical approaches stereo when lat -> +/- 90 */
        /* Also to avoid division by 0 for Mercator when lat -> +/- 90 */
  }

  Depth = 3*((int)(log_2(scale*Height)))+6;

  r1 = rseed;

  r1 = rand2(r1,r1);
  r2 = rand2(r1,r1);
  r3 = rand2(r1,r2);
  r4 = rand2(r2,r3);

  tetra[0].s = r1;
  tetra[1].s = r2;
  tetra[2].s = r3;
  tetra[3].s = r4;

  tetra[0].h = M;
  tetra[1].h = M;
  tetra[2].h = M;
  tetra[3].h = M;

  tetra[0].shadow = 0.0;
  tetra[1].shadow = 0.0;
  tetra[2].shadow = 0.0;
  tetra[3].shadow = 0.0;

  switch (view) {

    case 'm': /* Mercator projection */
      mercator();
      break;

    case 'p': /* Peters projection (area preserving cylindrical) */
      peter();
      break;

    case 'q': /* Square projection (equidistant latitudes) */
      squarep();
      break;

    case 'M': /* Mollweide projection (area preserving) */
      mollweide();
      break;

    case 'S': /* Sinusoid projection (area preserving) */
      sinusoid();
      break;

    case 's': /* Stereographic projection */
      stereo();
      break;

    case 'o': /* Orthographic projection */
      orthographic();
      break;

    case 'g': /* Gnomonic projection */
      gnomonic();
      break;

    case 'i': /* Icosahedral projection */
      icosahedral();
      break;

    case 'a': /* Area preserving azimuthal projection */
      azimuth();
      break;

    case 'c': /* Conical projection (conformal) */
      conical();
      break;

    case 'h': /* heightfield (obsolete) */
      orthographic();
      break;
  }

  if (do_outline) makeoutline(do_bw);

  if (vgrid != 0.0) { /* draw longitudes */
    int i,j;
    for (i=0; i<Width-1; i++)
      for (j=0; j<Height-1; j++) {
        double t;
        int g = 0;
        if (fabs(yyy[i][j])==1.0) g=1;
        else {
          t = floor((atan2(xxx[i][j],zzz[i][j])*180/PI+360)/vgrid);
          if (t != floor((atan2(xxx[i+1][j],zzz[i+1][j])*180/PI+360)/vgrid))
            g=1;
          if (t != floor((atan2(xxx[i][j+1],zzz[i][j+1])*180/PI+360)/vgrid))
            g=1;
        }
        if (g) {
          if (do_bw) col[i][j] = 0; else col[i][j] = GRID;
          if (doshade>0) shades[i][j] = 255;
        }
      }
  }

  if (hgrid != 0.0) { /* draw latitudes */
    int i,j;
    for (i=0; i<Width-1; i++)
      for (j=0; j<Height-1; j++) {
        double t;
        int g = 0;
        t = floor((asin(yyy[i][j])*180/PI+360)/hgrid);
        if (t != floor((asin(yyy[i+1][j])*180/PI+360)/hgrid))
          g=1;
        if (t != floor((asin(yyy[i][j+1])*180/PI+360)/hgrid))
          g=1;
        if (g) {
          if (do_bw) col[i][j] = 0; else col[i][j] = GRID;
          if (doshade>0) shades[i][j] = 255;
        }
      }
  }

  if (doshade>0) smoothshades();

  /* plot picture */
  switch (file_type)
  {
    case ppm:
      if (do_bw) printppmBW(outfile);
      else printppm(outfile);
      break;
    case xpm:
      if (do_bw) printxpmBW(outfile);
      else printxpm(outfile);
      break;
    case bmp:
      if (do_bw) printbmpBW(outfile);
      else printbmp(outfile);
      break;
    case heightfield:
      printheights(outfile);
      break;
  }

  return(0);
}

void readcolors(FILE *colfile, char* colorsname, char* biocolorsname)
{
  int crow, cNum = 0, oldcNum, i;

  if (NULL == (colfile = fopen(colorsname, "r")))
    {
      fprintf(stderr,
              "Cannot open %s\n",
              colorsname);
      exit(1);
    }


  /* Format of colour file is a sequence of lines       */
  /* each consisting of four integers:                  */
  /*   colour_number red green blue                     */
  /* where 0 <= colour_number <= 65535                  */
  /* and 0<= red, green, blue <= 255                    */
  /* The colour numbers must be increasing              */
  /* The first colours have special uses:               */
  /* 0 is usually black (0,0,0)                         */
  /* 1 is usually white (255,255,255)                   */
  /* 2 is the background colour                         */
  /* 3 is used for latitude/longitude grid lines        */
  /* 4 and 5 are used for outlines and contour lines    */
  /* 6 upwards are used for altitudes                   */
  /* Halfway between 6 and the max colour is sea level  */
  /* Shallowest sea is (max+6)/2 and land is above this */
  /* With 65536 colours, (max+6)/2 = 32770              */
  /* Colours between specified are interpolated         */

  for (crow = 0; !feof(colfile); crow++)
    {
      int       rValue, 
                gValue, 
                bValue,
                result = 0;

      oldcNum = cNum;  /* remember last colour number */
      result = fscanf(colfile, " %d %d %d %d",
                      &cNum, &rValue, &gValue, &bValue);

      if (result > 0)
        {
          if (cNum < oldcNum) cNum = oldcNum;
          if (cNum > 65535) cNum = 65535;
          rtable[cNum] = rValue;
          gtable[cNum] = gValue;
          btable[cNum] = bValue;
          /* interpolate colours between oldcNum and cNum */
          for (i = oldcNum+1; i<cNum; i++) {
            rtable[i] = (rtable[oldcNum]*(cNum-i)+rtable[cNum]*(i-oldcNum))
                        / (cNum-oldcNum);
            gtable[i] = (gtable[oldcNum]*(cNum-i)+gtable[cNum]*(i-oldcNum))
                        / (cNum-oldcNum);
            btable[i] = (btable[oldcNum]*(cNum-i)+btable[cNum]*(i-oldcNum))
                        / (cNum-oldcNum);
          }
        }
    }

  fclose(colfile);

  nocols = cNum+1;
  if (nocols < 10) nocols = 10;

  HIGHEST = nocols - 1;
  SEA = (HIGHEST+LOWEST)/2;
  LAND = SEA+1;

  for (i = cNum+1; i<nocols; i++) {
    /* fill up rest of colour table with last read colour */
    rtable[i] = rtable[cNum];
    gtable[i] = gtable[cNum];
    btable[i] = btable[cNum];
  }

  if (makeBiomes) {
  /* set default biome colours */
  rtable['I'-64+LAND] = 255;
  gtable['I'-64+LAND] = 255;
  btable['I'-64+LAND] = 255;
  rtable['T'-64+LAND] = 210;
  gtable['T'-64+LAND] = 210;
  btable['T'-64+LAND] = 210;
  rtable['G'-64+LAND] = 250;
  gtable['G'-64+LAND] = 215;
  btable['G'-64+LAND] = 165;
  rtable['B'-64+LAND] = 105;
  gtable['B'-64+LAND] = 155;
  btable['B'-64+LAND] = 120;
  rtable['D'-64+LAND] = 220;
  gtable['D'-64+LAND] = 195;
  btable['D'-64+LAND] = 175;
  rtable['S'-64+LAND] = 225;
  gtable['S'-64+LAND] = 155;
  btable['S'-64+LAND] = 100;
  rtable['F'-64+LAND] = 155;
  gtable['F'-64+LAND] = 215;
  btable['F'-64+LAND] = 170;
  rtable['R'-64+LAND] = 170;
  gtable['R'-64+LAND] = 195;
  btable['R'-64+LAND] = 200;
  rtable['W'-64+LAND] = 185;
  gtable['W'-64+LAND] = 150;
  btable['W'-64+LAND] = 160;
  rtable['E'-64+LAND] = 130;
  gtable['E'-64+LAND] = 190;
  btable['E'-64+LAND] = 25;
  rtable['O'-64+LAND] = 110;
  gtable['O'-64+LAND] = 160;
  btable['O'-64+LAND] = 170;

    if (NULL != (colfile = fopen(biocolorsname, "r")))
      for (crow = 0; !feof(colfile); crow++)
	{
	  char letter;
	  int rValue,  gValue,  bValue, result = 0;
	  result = fscanf(colfile, " %c %d %d %d",
			  &letter, &rValue, &gValue, &bValue);
	  
	  if (result > 0 && strchr("ITGBDSFRWEO",letter)>0) {
	    rtable[letter-64+LAND] = rValue;
	    gtable[letter-64+LAND] = gValue;
	    btable[letter-64+LAND] = bValue;
	  }
	}
  }
}

void makeoutline(int do_bw)
{
  int i,j,k,t;

  outx = (int*)calloc(Width*Height,sizeof(int));
  outy = (int*)calloc(Width*Height,sizeof(int));
  k=0;
  for (i=1; i<Width-1; i++)
    for (j=1; j<Height-1; j++)
      if ((col[i][j] >= LOWEST && col[i][j] <= SEA) &&
          (col[i-1][j] >= LAND || col[i+1][j] >= LAND ||
           col[i][j-1] >= LAND || col[i][j+1] >= LAND ||
           col[i-1][j-1] >= LAND || col[i-1][j+1] >= LAND ||
           col[i+1][j-1] >= LAND || col[i+1][j+1] >= LAND)) {
        /* if point is sea and any neighbour is not, add to outline */
        outx[k] = i; outy[k++] = j;
      }

  int contourstep = 0;

  if (contourLines > 0) {
    contourstep = (HIGHEST-LAND)/(contourLines+1);
    for (i=1; i<Width-1; i++)
      for (j=1; j<Height-1; j++) {
	t = (col[i][j] - LAND) / contourstep;
	if (col[i][j] >= LAND &&
	    ((col[i-1][j]-LAND) / contourstep > t ||
	     (col[i+1][j]-LAND) / contourstep > t ||
	     (col[i][j-1]-LAND) / contourstep > t ||
	     (col[i][j+1]-LAND) / contourstep > t)) {
	  /* if point is at contour line and any neighbour is higher */
	  outx[k] = i; outy[k++] = j;
	}
      }
  }
  if (coastContourLines > 0) {
    contourstep = (LAND-LOWEST)/20;
    for (i=1; i<Width-1; i++)
      for (j=1; j<Height-1; j++) {
	t = (col[i][j] - LAND) / contourstep;
	if (col[i][j] <= SEA && t >= -coastContourLines &&
	    ((col[i-1][j]-LAND) / contourstep > t ||
	     (col[i+1][j]-LAND) / contourstep > t ||
	     (col[i][j-1]-LAND) / contourstep > t ||
	     (col[i][j+1]-LAND) / contourstep > t)) {
	  /* if point is at contour line and any neighbour is higher */
	  outx[k] = i; outy[k++] = j;
	}
      }
  }
  if (do_bw) /* if outline only, clear colours */
    for (i=0; i<Width; i++)
      for (j=0; j<Height; j++) {
        if (col[i][j] >= LOWEST)
          col[i][j] = WHITE;
        else col[i][j] = BLACK;
      }
  /* draw outline (in black if outline only) */
  contourstep = (HIGHEST-LAND)/(contourLines+1);
  for (j = 0; j < k; j++) {
    if (do_bw) t = BLACK;
    else {
      t = col[outx[j]][outy[j]];
      if (t != OUTLINE1 && t != OUTLINE2) {
	if (contourLines > 0 && t >= LAND)
	  if (((t-LAND)/contourstep)%2 == 1)
	    t = OUTLINE1;
	  else t = OUTLINE2;
	else if (t <= SEA)
	  t = OUTLINE1;
      }
    }
    col[outx[j]][outy[j]] = t;
  }
}

void readmap()  /* reads in a map for matching */
{
  int i,j = 0, step = 1;
  double y;
  char c, cs[1000];
  int Width, Height = 0;

  do {
    scanf("%255[^\n]\n",cs); /* read line */
    /* fprintf(stderr,"%s\n",cs); */
  
    Width = strlen(cs);
    if (Width<48) {
      MatchWidth = 2*Width;
      step = 2;
    }
    else MatchWidth = Width;

    for(i = 0; i < MatchWidth ; i+=step) {
      c = cs[i/step];
      switch (c) {
      case '.': cl0[i][j] = -8;
                break;
      case ',': cl0[i][j] = -6;
                break;
      case ':': cl0[i][j] = -4;
                break;
      case ';': cl0[i][j] = -2;
                break;
      case '-': cl0[i][j] = 0;
                break;
      case '*': cl0[i][j] = 2;
                break;
      case 'o': cl0[i][j] = 4;
                break;
      case 'O': cl0[i][j] = 6;
                break;
      case '@': cl0[i][j] = 8;
                break;
      default: fprintf(stderr,"Wrong map symbol: %c\n",c);
      }
    }
    Height++;
    j += step;
  }
  while (!feof(stdin));
  
  MatchHeight = step*Height;

  /* fprintf(stderr,"%d %d\n",MatchWidth,MatchHeight); */

  if (step==2) {
    /* interpolate */
    for (j = 1; j < MatchHeight; j+=2)
      for(i = 0; i < MatchWidth ; i+=2)
	cl0[i][j] = (cl0[i][j-1] + cl0[i][(j+1)])/2;
    for (j = 0; j < MatchHeight; j++)
      for(i = 1; i < MatchWidth ; i+=2)
	cl0[i][j] = (cl0[i-1][j] + cl0[(i+1)%MatchWidth][j])/2;
  }
}


void smoothshades()
{
  int i,j;

  for (i=0; i<Width-2; i++)
    for (j=0; j<Height-2; j++)
      shades[i][j] = (4*shades[i][j]+2*shades[i][j+1]
                      +2*shades[i+1][j]+shades[i+1][j+1]+4)/9;
}

void mercator()
{
  double y,scale1,cos2,theta1;
  int i,j,k;
  void planet0();

  y = sin(lat);
  y = (1.0+y)/(1.0-y);
  y = 0.5*log(y);
  k = (int)(0.5*y*Width*scale/PI+0.5);
  for (j = 0; j < Height; j++) {
    y = PI*(2.0*(j-k)-Height)/Width/scale;
    y = exp(2.*y);
    y = (y-1.)/(y+1.);
    scale1 = scale*Width/Height/sqrt(1.0-y*y)/PI;
    cos2 = sqrt(1.0-y*y);
    Depth = 3*((int)(log_2(scale1*Height)))+3;
    for (i = 0; i < Width ; i++) {
      theta1 = longi-0.5*PI+PI*(2.0*i-Width)/Width/scale;
      planet0(cos(theta1)*cos2,y,-sin(theta1)*cos2, i,j);
    }
  }
}

void peter()
{
  double y,cos2,theta1,scale1;
  int k,i,j,water,land;
  void planet0();

  y = 2.0*sin(lat);
  k = (int)(0.5*y*Width*scale/PI+0.5);
  water = land = 0;
  for (j = 0; j < Height; j++) {
    y = 0.5*PI*(2.0*(j-k)-Height)/Width/scale;
    if (fabs(y)>1.0)
      for (i = 0; i < Width ; i++) {
        col[i][j] = BACK;
        if (doshade>0) shades[i][j] = 255;
      }
    else {
      cos2 = sqrt(1.0-y*y);
      if (cos2>0.0) {
        scale1 = scale*Width/Height/cos2/PI;
        Depth = 3*((int)(log_2(scale1*Height)))+3;
        for (i = 0; i < Width ; i++) {
          theta1 = longi-0.5*PI+PI*(2.0*i-Width)/Width/scale;
          planet0(cos(theta1)*cos2,y,-sin(theta1)*cos2, i,j);
          if (col[i][j] < LAND) water++; else land++;
        }
      }
    }
  }
  fprintf(stderr,"water percentage: %d\n",100*water/(water+land));
}

void squarep()
{
  double y,scale1,theta1,cos2;
  int k,i,j;
  void planet0();

  k = (int)(0.5*lat*Width*scale/PI+0.5);
  for (j = 0; j < Height; j++) {
    y = (2.0*(j-k)-Height)/Width/scale*PI;
    if (fabs(y+y)>PI)
      for (i = 0; i < Width ; i++) {
        col[i][j] = BACK;
      if (doshade>0) shades[i][j] = 255;
    } else {
      cos2 = cos(y);
      if (cos2>0.0) {
        scale1 = scale*Width/Height/cos2/PI;
        Depth = 3*((int)(log_2(scale1*Height)))+3;
        for (i = 0; i < Width ; i++) {
          theta1 = longi-0.5*PI+PI*(2.0*i-Width)/Width/scale;
          planet0(cos(theta1)*cos2,sin(y),-sin(theta1)*cos2, i,j);
        }
      }
    }
  }
}

void mollweide()
{
  double y,y1,zz,scale1,cos2,theta1,theta2;
  int i,j,i1=1,k;
  void planet0();

  for (j = 0; j < Height; j++) {
    y1 = 2*(2.0*j-Height)/Width/scale;
    if (fabs(y1)>=1.0) for (i = 0; i < Width ; i++) {
      col[i][j] = BACK;
      if (doshade>0) shades[i][j] = 255;
    } else {
      zz = sqrt(1.0-y1*y1);
      y = 2.0/PI*(y1*zz+asin(y1));
      cos2 = sqrt(1.0-y*y);
      if (cos2>0.0) {
        scale1 = scale*Width/Height/cos2/PI;
        Depth = 3*((int)(log_2(scale1*Height)))+3;
        for (i = 0; i < Width ; i++) {
          theta1 = PI/zz*(2.0*i-Width)/Width/scale;
          if (fabs(theta1)>PI) {
            col[i][j] = BACK;
            if (doshade>0) shades[i][j] = 255;
          } else {
            double x2,y2,z2, x3,y3,z3;
            theta1 += -0.5*PI;
            x2 = cos(theta1)*cos2;
            y2 = y;
            z2 = -sin(theta1)*cos2;
            x3 = clo*x2+slo*sla*y2+slo*cla*z2;
            y3 = cla*y2-sla*z2;
            z3 = -slo*x2+clo*sla*y2+clo*cla*z2;

            planet0(x3,y3,z3, i,j);
          }
        }
      }
    }
  }
}

void sinusoid()
{
  double y,theta1,theta2,cos2,l1,i1,scale1;
  int k,i,j,l,c;
  void planet0();

  k = (int)(lat*Width*scale/PI+0.5);
  for (j = 0; j < Height; j++) {
    y = (2.0*(j-k)-Height)/Width/scale*PI;
    if (fabs(y+y)>PI) for (i = 0; i < Width ; i++) {
      col[i][j] = BACK;
      if (doshade>0) shades[i][j] = 255;
    } else {
      cos2 = cos(y);
      if (cos2>0.0) {
        scale1 = scale*Width/Height/cos2/PI;
        Depth = 3*((int)(log_2(scale1*Height)))+3;
        for (i = 0; i<Width; i++) {
          l = i*12/Width/scale;
          l1 = l*Width*scale/12.0;
          i1 = i-l1;
          theta2 = longi-0.5*PI+PI*(2.0*l1-Width)/Width/scale;
          theta1 = (PI*(2.0*i1-Width*scale/12.0)/Width/scale)/cos2;
          if (fabs(theta1)>PI/12.0) {
            col[i][j] = BACK;
            if (doshade>0) shades[i][j] = 255;
          } else {
            planet0(cos(theta1+theta2)*cos2,sin(y),-sin(theta1+theta2)*cos2,
                    i,j);
          }
        }
      }
    }
  }
}

void stereo()
{
  double x,y,z,zz,x1,y1,z1,theta1,theta2;
  int i,j;
  double scale1;
  void planet0();

  for (j = 0; j < Height; j++) {
    for (i = 0; i < Width ; i++) {
      x = (2.0*i-Width)/Height/scale;
      y = (2.0*j-Height)/Height/scale;
      z = x*x+y*y;
      zz = 0.25*(4.0+z);
      x = x/zz;
      y = y/zz;
      z = (1.0-0.25*z)/zz;
      x1 = clo*x+slo*sla*y+slo*cla*z;
      y1 = cla*y-sla*z;
      z1 = -slo*x+clo*sla*y+clo*cla*z;
      if (scale<1.0) Depth = 3*((int)(log_2(scale*Height)))+6+1.5/scale;

      planet0(x1,y1,z1, i,j);
    }
  }
}

void orthographic()
{
  double x,y,z,x1,y1,z1,theta1,theta2,zz;
  int i,j;
  void planet0();

  for (j = 0; j < Height; j++) {
    for (i = 0; i < Width ; i++) {
      x = (2.0*i-Width)/Height/scale;
      y = (2.0*j-Height)/Height/scale;
      if (x*x+y*y>1.0) {
        col[i][j] = BACK;
        if (doshade>0) shades[i][j] = 255;
      } else {
        z = sqrt(1.0-x*x-y*y);
        x1 = clo*x+slo*sla*y+slo*cla*z;
        y1 = cla*y-sla*z;
        z1 = -slo*x+clo*sla*y+clo*cla*z;
        planet0(x1,y1,z1, i,j);
      }
    }
  }
}

void icosahedral() /* modified version of gnomonic */
{
  double x,y,z,x1,y1,z1,zz,theta1,theta2;
  int i,j;
  void planet0();
  double lat1, longi1, sla, cla, slo, clo, x0, y0, sq3_4, sq3;
  double L1, L2, S;

  sq3 = sqrt(3.0);
  L1 =  10.812317; /* theoretically 10.9715145571469; */
  L2 = -52.622632; /* theoretically -48.3100310579607; */
  S = 55.6; /* found by experimentation */
  for (j = 0; j < Height; j++) {
    for (i = 0; i < Width ; i++) {

      x0 = 198.0*(2.0*i-Width)/Width/scale-36;
      y0 = 198.0*(2.0*j-Height)/Width/scale - lat/DEG2RAD;

      longi1 = 0.0;
      lat1 = 500.0;
      if (y0/sq3 <= 18.0 && y0/sq3 >= -18.0) { /* middle row of triangles */
        /* upward triangles */
        if (x0-y0/sq3 < 144.0 && x0+y0/sq3 >= 108.0) {
          lat1 = -L1;
          longi1 = 126.0;
        }
        else if (x0-y0/sq3 < 72.0 && x0+y0/sq3 >= 36.0) {
          lat1 = -L1;
          longi1 = 54.0;
        }
        else if (x0-y0/sq3 < 0.0 && x0+y0/sq3 >= -36.0) {
          lat1 = -L1;
          longi1 = -18.0;
        }
        else if (x0-y0/sq3 < -72.0 && x0+y0/sq3 >= -108.0) {
          lat1 = -L1;
          longi1 = -90.0;
        }
        else if (x0-y0/sq3 < -144.0 && x0+y0/sq3 >= -180.0) {
          lat1 = -L1;
          longi1 = -162.0;
        }

        /* downward triangles */
        else if (x0+y0/sq3 < 108.0 && x0-y0/sq3 >= 72.0) {
          lat1 = L1;
          longi1 = 90.0;
        }
        else if (x0+y0/sq3 < 36.0 && x0-y0/sq3 >= 0.0) {
          lat1 = L1;
          longi1 = 18.0;
        }
        else if (x0+y0/sq3 < -36.0 && x0-y0/sq3 >= -72.0) {
          lat1 = L1;
          longi1 = -54.0;
        }
        else if (x0+y0/sq3 < -108.0 && x0-y0/sq3 >= -144.0) {
          lat1 = L1;
          longi1 = -126.0;
        }
        else if (x0+y0/sq3 < -180.0 && x0-y0/sq3 >= -216.0) {
          lat1 = L1;
          longi1 = -198.0;
        }
      }

      if (y0/sq3 > 18.0) { /* bottom row of triangles */
        if (x0+y0/sq3 < 180.0 && x0-y0/sq3 >= 72.0) {
          lat1 = L2;
          longi1 = 126.0;
        }
        else if (x0+y0/sq3 < 108.0 && x0-y0/sq3 >= 0.0) {
          lat1 = L2;
          longi1 = 54.0;
        }
        else if (x0+y0/sq3 < 36.0 && x0-y0/sq3 >= -72.0) {
          lat1 = L2;
          longi1 = -18.0;
        }
        else if (x0+y0/sq3 < -36.0 && x0-y0/sq3 >= -144.0) {
          lat1 = L2;
          longi1 = -90.0;
        }
        else if (x0+y0/sq3 < -108.0 && x0-y0/sq3 >= -216.0) {
          lat1 = L2;
          longi1 = -162.0;
        }
      }
      if (y0/sq3 < -18.0) { /* top row of triangles */
        if (x0-y0/sq3 < 144.0 && x0+y0/sq3 >= 36.0) {
          lat1 = -L2;
          longi1 = 90.0;
        }
        else if (x0-y0/sq3 < 72.0 && x0+y0/sq3 >= -36.0) {
          lat1 = -L2;
          longi1 = 18.0;
        }
        else if (x0-y0/sq3 < 0.0 && x0+y0/sq3 >= -108.0) {
          lat1 = -L2;
          longi1 = -54.0;
        }
        else if (x0-y0/sq3 < -72.0 && x0+y0/sq3 >= -180.0) {
          lat1 = -L2;
          longi1 = -126.0;
        }
        else if (x0-y0/sq3 < -144.0 && x0+y0/sq3 >= -252.0) {
          lat1 = -L2;
          longi1 = -198.0;
        }
      }

      if (lat1 > 400.0) {
        col[i][j] = BACK;
        if (doshade>0) shades[i][j] = 255;
      } else {
        x = (x0 - longi1)/S;
        y = (y0 + lat1)/S;

        longi1 = longi1*DEG2RAD - longi;
        lat1 = lat1*DEG2RAD;

        sla = sin(lat1); cla = cos(lat1);
        slo = sin(longi1); clo = cos(longi1);

        zz = sqrt(1.0/(1.0+x*x+y*y));
        x = x*zz;
        y = y*zz;
        z = sqrt(1.0-x*x-y*y);
        x1 = clo*x+slo*sla*y+slo*cla*z;
        y1 = cla*y-sla*z;
        z1 = -slo*x+clo*sla*y+clo*cla*z;

        planet0(x1,y1,z1, i,j);
      }
    }
  }
}

void gnomonic()
{
  double x,y,z,x1,y1,z1,zz,theta1,theta2;
  int i,j;
  void planet0();

  if (scale<1.0) Depth = 3*((int)(log_2(scale*Height)))+6+1.5/scale;
  for (j = 0; j < Height; j++) {
    for (i = 0; i < Width ; i++) {
      x = (2.0*i-Width)/Height/scale;
      y = (2.0*j-Height)/Height/scale;
      zz = sqrt(1.0/(1.0+x*x+y*y));
      x = x*zz;
      y = y*zz;
      z = sqrt(1.0-x*x-y*y);
      x1 = clo*x+slo*sla*y+slo*cla*z;
      y1 = cla*y-sla*z;
      z1 = -slo*x+clo*sla*y+clo*cla*z;
      planet0(x1,y1,z1, i,j);
    }
  }
}

void azimuth()
{
  double x,y,z,x1,y1,z1,zz,theta1,theta2;
  int i,j;
  void planet0();

  for (j = 0; j < Height; j++) {
    for (i = 0; i < Width ; i++) {
      x = (2.0*i-Width)/Height/scale;
      y = (2.0*j-Height)/Height/scale;
      zz = x*x+y*y;
      z = 1.0-0.5*zz;
      if (z<-1.0) {
        col[i][j] = BACK;
        if (doshade>0) shades[i][j] = 255;
      } else {
        zz = sqrt(1.0-0.25*zz);
        x = x*zz;
        y = y*zz;
        x1 = clo*x+slo*sla*y+slo*cla*z;
        y1 = cla*y-sla*z;
        z1 = -slo*x+clo*sla*y+clo*cla*z;
        planet0(x1,y1,z1, i,j);
      }
    }
  }
}

void conical()
{
  double k1,c,y2,x,y,zz,x1,y1,z1,theta1,theta2,cos2;
  int i,j;
  void planet0();

  if (scale<1.0) Depth = 3*((int)(log_2(scale*Height)))+6+1.5/scale;

  if (lat>0) {
    k1 = 1.0/sin(lat);
    c = k1*k1;
    y2 = sqrt(c*(1.0-sin(lat/k1))/(1.0+sin(lat/k1)));
    for (j = 0; j < Height; j++) {
      for (i = 0; i < Width ; i++) {
        x = (2.0*i-Width)/Height/scale;
        y = (2.0*j-Height)/Height/scale+y2;
        zz = x*x+y*y;
        if (zz==0.0) theta1 = 0.0; else theta1 = k1*atan2(x,y);
        if (theta1<-PI || theta1>PI) {
          col[i][j] = BACK;
          if (doshade>0) shades[i][j] = 255;
        } else {
          theta1 += longi-0.5*PI; /* theta1 is longitude */
          theta2 = k1*asin((zz-c)/(zz+c)); /* theta2 is latitude */
          if (theta2 > 0.5*PI || theta2 < -0.5*PI) {
            col[i][j] = BACK;
            if (doshade>0) shades[i][j] = 255;
          } else {
            cos2 = cos(theta2);
            y = sin(theta2);
            planet0(cos(theta1)*cos2,y,-sin(theta1)*cos2, i, j);
          }
        }
      }
    }
  }
  else {
    k1 = 1.0/sin(lat);
    c = k1*k1;
    y2 = sqrt(c*(1.0-sin(lat/k1))/(1.0+sin(lat/k1)));
    for (j = 0; j < Height; j++) {
      for (i = 0; i < Width ; i++) {
        x = (2.0*i-Width)/Height/scale;
        y = (2.0*j-Height)/Height/scale-y2;
        zz = x*x+y*y;
        if (zz==0.0) theta1 = 0.0; else theta1 = -k1*atan2(x,-y);
        if (theta1<-PI || theta1>PI) {
          col[i][j] = BACK;
          if (doshade>0) shades[i][j] = 255;
        } else {
          theta1 += longi-0.5*PI; /* theta1 is longitude */
          theta2 = k1*asin((zz-c)/(zz+c)); /* theta2 is latitude */
          if (theta2 > 0.5*PI || theta2 < -0.5*PI) {
            col[i][j] = BACK;
            if (doshade>0) shades[i][j] = 255;
          } else {
            cos2 = cos(theta2);
            y = sin(theta2);
            planet0(cos(theta1)*cos2,y,-sin(theta1)*cos2, i, j);
          }
        }
      }
    }
  }
}

double rand2(p,q) /* random number generator taking two seeds */
double p,q;       /* rand2(p,q) = rand2(q,p) is important     */
{
  double r;
  r = (p+3.14159265)*(q+3.14159265);
  return(2.*(r-(int)r)-1.);
}

void planet0(x,y,z, i, j)
double x,y,z;
int i, j;
{
  double alt, planet1(), y2, sun, temp, rain;
  int colour;

  alt = planet1(x,y,z);

  /* calculate temperature based on altitude and latitude */
  /* scale: -0.1 to 0.1 corresponds to -30 to +30 degrees Celsius */
  sun = sqrt(1.0-y*y); /* approximate amount of sunlight at
			     latitude ranged from 0.1 to 1.1 */
  if (alt < 0) temp = sun/8.0 + alt*0.3; /* deep water colder */
  else temp = sun/8.0 - alt*1.2; /* high altitudes colder */

  if (temp<tempMin && alt >0) tempMin = temp;
  if (temp>tempMax && alt >0) tempMax = temp;
  if (temperature) alt = temp-0.05;

  /* calculate rainfall based on temperature and latitude */
  /* rainfall approximately proportional to temperature but reduced
     near horse latitudes (+/- 30 degrees, y=0.5) and reduced for
     rain shadow */
  y2 = fabs(y)-0.5;
  rain = temp*0.65 + 0.1 - 0.011/(y2*y2+0.1);
  rain += 0.03*rainShadow;
  if (rain<0.0) rain = 0.0;

  if (rain<rainMin && alt >0) rainMin = rain;
  if (rain>rainMax && alt >0) rainMax = rain;
  
  if (rainfall) alt = rain-0.02;

  if (nonLinear) {
    /* non-linear scaling to make flatter near sea level */
    alt = alt*alt*alt * 300;
  }
  /* store height for heightfield */
  if (file_type == heightfield) heights[i][j] = 10000000*alt;

  y2 = y*y; y2 = y2*y2; y2 = y2*y2;

  /* calculate colour */

  if (makeBiomes) { /* make biome colours */
    int tt = min(44,max(0,(int)(rain*300.0-9)));
    int rr = min(44,max(0,(int)(temp*300.0+10)));
    char bio = biomes[tt][rr];
    if (alt <= 0.0) {
	colour = SEA+(int)((SEA-LOWEST+1)*(10*alt));
	if (colour<LOWEST) colour = LOWEST;
    }
    else colour = bio-64+LAND;  /* from LAND+2 to LAND+23 */
  }  
  else if (alt <=0.0) { /* if below sea level then */
    if (latic > 0 && y2+alt >= 1.0 - 0.02*latic*latic)
      colour = HIGHEST;  /* icecap if close to poles */
    else {
      colour = SEA+(int)((SEA-LOWEST+1)*(10*alt));
      if (colour<LOWEST) colour = LOWEST;
    }
  }
  else {
    if (latic) alt += 0.1*latic*y2;  /* altitude adjusted with latitude */
    if (alt >= 0.1) /* if high then */
      colour = HIGHEST;
    else {
      colour = LAND+(int)((HIGHEST-LAND+1)*(10*alt));
      if (colour>HIGHEST) colour = HIGHEST;
    }
  }

  /* store colour */
  col[i][j] = colour;

  /* store (x,y,z) coordinates for grid drawing */
  if (vgrid != 0.0) {
    xxx[i][j] = x;
    zzz[i][j] = z;
  }
  if (hgrid != 0.0 || vgrid != 0.0) yyy[i][j] = y;

  /* store shading info */
  if (doshade>0) shades[i][j] = shade;

  return;
}

vertex ssa, ssb, ssc, ssd;

double planet(a,b,c,d, x,y,z, level)
vertex a,b,c,d;             /* tetrahedron vertices */
double x,y,z;               /* goal point */
int level;                  /* levels to go */
{
  vertex e;
  double lab, lac, lad, lbc, lbd, lcd, maxlength;
  double es1, es2, es3;
  double eax,eay,eaz, epx,epy,epz;
  double ecx,ecy,ecz, edx,edy,edz;
  double x1,y1,z1,x2,y2,z2,l1,tmp;

  if (level>0) {

    /* make sure ab is longest edge */
    lab = dist2(a,b);
    lac = dist2(a,c);
    lad = dist2(a,d);
    lbc = dist2(b,c);
    lbd = dist2(b,d);
    lcd = dist2(c,d);

    maxlength = lab;
    if (lac > maxlength) maxlength = lac;
    if (lad > maxlength) maxlength = lad;
    if (lbc > maxlength) maxlength = lbc;
    if (lbd > maxlength) maxlength = lbd;
    if (lcd > maxlength) maxlength = lcd;

    if (lac == maxlength) return(planet(a,c,b,d, x,y,z, level));
    if (lad == maxlength) return(planet(a,d,b,c, x,y,z, level));
    if (lbc == maxlength) return(planet(b,c,a,d, x,y,z, level));
    if (lbd == maxlength) return(planet(b,d,a,c, x,y,z, level));
    if (lcd == maxlength) return(planet(c,d,a,b, x,y,z, level));

    if (level == 11) { /* save tetrahedron for caching */
      ssa = a; ssb = b; ssc = c; ssd = d;
    }

    /* ab is longest, so cut ab */
      e.s = rand2(a.s,b.s);
      es1 = rand2(e.s,e.s);
      es2 = 0.5+0.1*rand2(es1,es1);  /* find cut point */
      es3 = 1.0-es2;

      if (a.s<b.s) {
        e.x = es2*a.x+es3*b.x; e.y = es2*a.y+es3*b.y; e.z = es2*a.z+es3*b.z;
      } else if (a.s>b.s) {
        e.x = es3*a.x+es2*b.x; e.y = es3*a.y+es2*b.y; e.z = es3*a.z+es2*b.z;
      } else { /* as==bs, very unlikely to ever happen */
        e.x = 0.5*a.x+0.5*b.x; e.y = 0.5*a.y+0.5*b.y; e.z = 0.5*a.z+0.5*b.z;
      }

      /* new altitude is: */
      if (matchMap && lab > matchSize) { /* use map height */
        double l, xx, yy;
        l = sqrt(e.x*e.x+e.y*e.y+e.z*e.z);
        yy = asin(e.y/l)*(MatchHeight-1)/PI+(MatchHeight-1)/2.0;
	xx = atan2(e.x,e.z)*(MatchWidth-1)/2.0/PI+(MatchWidth-1)/2.0;
        e.h = cl0[(int)(xx+0.5)][(int)(yy+0.5)]*0.1/8.0;
      } else {
        if (lab>1.0) lab = pow(lab,0.5);
        /* decrease contribution for very long distances */
        e.h = 0.5*(a.h+b.h) /* average of end points */
          + e.s*dd1*pow(fabs(a.h-b.h),POWA)
          /* plus contribution for altitude diff */
          + es1*dd2*pow(lab,POW); /* plus contribution for distance */
      }

      /* calculate approximate rain shadow for new point */
      if (e.h <= 0.0 || !(rainfall || makeBiomes)) e.shadow = 0.0;
      else {
      x1 = 0.5*(a.x+b.x);
      x1 = a.h*(x1-a.x)+b.h*(x1-b.x);
      y1 = 0.5*(a.y+b.y);
      y1 = a.h*(y1-a.y)+b.h*(y1-b.y);
      z1 = 0.5*(a.z+b.z);
      z1 = a.h*(z1-a.z)+b.h*(z1-b.z);
      l1 = sqrt(x1*x1+y1*y1+z1*z1);
      if (l1==0.0) l1 = 1.0;
      tmp = sqrt(1.0-y*y);
      if (tmp<0.0001) tmp = 0.0001;
      x2 = x*x1+y*y1+z*z1;
      z2 = -z/tmp*x1+x/tmp*z1;
      if (lab > 0.04)
	e.shadow = (a.shadow + b.shadow- cos(PI*shade_angle/180.0)*z2/l1)/3.0;
      else
	e.shadow = (a.shadow + b.shadow)/2.0;
      }
      


      /* find out in which new tetrahedron target point is */
      eax = a.x-e.x; eay = a.y-e.y; eaz = a.z-e.z;
      ecx = c.x-e.x; ecy = c.y-e.y; ecz = c.z-e.z;
      edx = d.x-e.x; edy = d.y-e.y; edz = d.z-e.z;
      epx =   x-e.x; epy =   y-e.y; epz =   z-e.z;
      if ((eax*ecy*edz+eay*ecz*edx+eaz*ecx*edy
           -eaz*ecy*edx-eay*ecx*edz-eax*ecz*edy)*
          (epx*ecy*edz+epy*ecz*edx+epz*ecx*edy
           -epz*ecy*edx-epy*ecx*edz-epx*ecz*edy)>0.0) {
        /* point is inside acde */
        return(planet(c,d,a,e, x,y,z, level-1));
      } else {
        /* point is inside bcde */
        return(planet(c,d,b,e, x,y,z, level-1));
      }
  }
  else { /* level == 0 */
    if (doshade==1 || doshade==2) { /* bump map */
      x1 = 0.25*(a.x+b.x+c.x+d.x);
      x1 = a.h*(x1-a.x)+b.h*(x1-b.x)+c.h*(x1-c.x)+d.h*(x1-d.x);
      y1 = 0.25*(a.y+b.y+c.y+d.y);
      y1 = a.h*(y1-a.y)+b.h*(y1-b.y)+c.h*(y1-c.y)+d.h*(y1-d.y);
      z1 = 0.25*(a.z+b.z+c.z+d.z);
      z1 = a.h*(z1-a.z)+b.h*(z1-b.z)+c.h*(z1-c.z)+d.h*(z1-d.z);
      l1 = sqrt(x1*x1+y1*y1+z1*z1);
      if (l1==0.0) l1 = 1.0;
      tmp = sqrt(1.0-y*y);
      if (tmp<0.0001) tmp = 0.0001;
      x2 = x*x1+y*y1+z*z1;
      y2 = -x*y/tmp*x1+tmp*y1-z*y/tmp*z1;
      z2 = -z/tmp*x1+x/tmp*z1;
      shade =
        (int)((-sin(PI*shade_angle/180.0)*y2-cos(PI*shade_angle/180.0)*z2)
              /l1*48.0+128.0);
      if (shade<10) shade = 10;
      if (shade>255) shade = 255;
      if (doshade==2 && (a.h+b.h+c.h+d.h)<0.0) shade = 150;
    }
    else if (doshade==3) { /* daylight shading */
      double hh = a.h+b.h+c.h+d.h;
      if (hh<=0.0) { /* sea */
        x1 = x; y1 = y; z1 = z; /* (x1,y1,z1) = normal vector */
      } else { /* add bumbmap effect */
        x1 = 0.25*(a.x+b.x+c.x+d.x);
        x1 = (a.h*(x1-a.x)+b.h*(x1-b.x)+c.h*(x1-c.x)+d.h*(x1-d.x));
        y1 = 0.25*(a.y+b.y+c.y+d.y);
        y1 = (a.h*(y1-a.y)+b.h*(y1-b.y)+c.h*(y1-c.y)+d.h*(y1-d.y));
        z1 = 0.25*(a.z+b.z+c.z+d.z);
        z1 = (a.h*(z1-a.z)+b.h*(z1-b.z)+c.h*(z1-c.z)+d.h*(z1-d.z));
	l1 = 5.0*sqrt(x1*x1+y1*y1+z1*z1);
	x1 += x*l1; y1 += y*l1; z1 += z*l1;
      }
      l1 = sqrt(x1*x1+y1*y1+z1*z1);
      if (l1==0.0) l1 = 1.0;
      x2 = cos(PI*shade_angle/180.0-0.5*PI)*cos(PI*shade_angle2/180.0);
      y2 = -sin(PI*shade_angle2/180.0);
      z2 = -sin(PI*shade_angle/180.0-0.5*PI)*cos(PI*shade_angle2/180.0);
      shade = (int)((x1*x2+y1*y2+z1*z2)/l1*170.0+10);
      if (shade<10) shade = 10;
      if (shade>255) shade = 255;
    }
    rainShadow  = 0.25*(a.shadow+b.shadow+c.shadow+d.shadow);
    return 0.25*(a.h+b.h+c.h+d.h);
  }
}

double planet1(x,y,z)
double x,y,z;
{
  vertex a,b,c,d;

  double abx,aby,abz, acx,acy,acz, adx,ady,adz, apx,apy,apz;
  double bax,bay,baz, bcx,bcy,bcz, bdx,bdy,bdz, bpx,bpy,bpz;
  
  /* check if point is inside cached tetrahedron */

  abx = ssb.x-ssa.x; aby = ssb.y-ssa.y; abz = ssb.z-ssa.z;
  acx = ssc.x-ssa.x; acy = ssc.y-ssa.y; acz = ssc.z-ssa.z;
  adx = ssd.x-ssa.x; ady = ssd.y-ssa.y; adz = ssd.z-ssa.z;
  apx = x-ssa.x; apy = y-ssa.y; apz = z-ssa.z;
  
  if ((adx*aby*acz+ady*abz*acx+adz*abx*acy
       -adz*aby*acx-ady*abx*acz-adx*abz*acy)*
      (apx*aby*acz+apy*abz*acx+apz*abx*acy
       -apz*aby*acx-apy*abx*acz-apx*abz*acy) > 0.0) {
    /* p is on same side of abc as d */
    if ((acx*aby*adz+acy*abz*adx+acz*abx*ady
         -acz*aby*adx-acy*abx*adz-acx*abz*ady)*
        (apx*aby*adz+apy*abz*adx+apz*abx*ady
         -apz*aby*adx-apy*abx*adz-apx*abz*ady) > 0.0) {
      /* p is on same side of abd as c */
      if ((abx*ady*acz+aby*adz*acx+abz*adx*acy
           -abz*ady*acx-aby*adx*acz-abx*adz*acy)*
          (apx*ady*acz+apy*adz*acx+apz*adx*acy
           -apz*ady*acx-apy*adx*acz-apx*adz*acy) > 0.0) {
        /* p is on same side of acd as b */
        bax = -abx; bay = -aby; baz = -abz;
        bcx = ssc.x-ssb.x; bcy = ssc.y-ssb.y; bcz = ssc.z-ssb.z;
        bdx = ssd.x-ssb.x; bdy = ssd.y-ssb.y; bdz = ssd.z-ssb.z;
        bpx = x-ssb.x; bpy = y-ssb.y; bpz = z-ssb.z;
        if ((bax*bcy*bdz+bay*bcz*bdx+baz*bcx*bdy
             -baz*bcy*bdx-bay*bcx*bdz-bax*bcz*bdy)*
            (bpx*bcy*bdz+bpy*bcz*bdx+bpz*bcx*bdy
             -bpz*bcy*bdx-bpy*bcx*bdz-bpx*bcz*bdy) > 0.0) {
          /* p is on same side of bcd as a */
          /* Hence, p is inside cached tetrahedron */
          /* so we start from there */
          return(planet(ssa,ssb,ssc,ssd, x,y,z, 11));
        }
      }
    }
  }
 /* otherwise, we start from scratch */

  return(planet(tetra[0], tetra[1], tetra[2], tetra[3],
                         /* vertices of tetrahedron */
                x,y,z,   /* coordinates of point we want colour of */
                Depth)); /* subdivision depth */

}


void printppm(outfile) /* prints picture in PPM (portable pixel map) format */
FILE *outfile;
{
  int i,j,c,s;

  fprintf(outfile,"P6\n");
  fprintf(outfile,"#fractal planet image\n");
  fprintf(outfile,"# Command line:\n# %s\n",cmdLine);
  fprintf(outfile,"%d %d 255\n",Width,Height);

  if (doshade) {
    for (j=0; j<Height; j++) {
      for (i=0; i<Width; i++) {
        s =shades[i][j];
        c = s*rtable[col[i][j]]/150;
        if (c>255) c=255;
        putc(c,outfile);
        c = s*gtable[col[i][j]]/150;
        if (c>255) c=255;
        putc(c,outfile);
        c = s*btable[col[i][j]]/150;
        if (c>255) c=255;
        putc(c,outfile);
      }
    }
  } else {
    for (j=0; j<Height; j++)
      for (i=0; i<Width; i++) {
        putc(rtable[col[i][j]],outfile);
        putc(gtable[col[i][j]],outfile);
        putc(btable[col[i][j]],outfile);
      }
  }
  fclose(outfile);
}

void printppmBW(outfile) /* prints picture in b/w PPM format */
FILE *outfile;
{
  int i,j,c;

  fprintf(outfile,"P6\n");
  fprintf(outfile,"#fractal planet image\n");
  fprintf(outfile,"# Command line:\n# %s\n",cmdLine);
  fprintf(outfile,"%d %d 1\n",Width,Height);

  for (j=0; j<Height; j++)
    for (i=0; i<Width; i++) {
      if (col[i][j] < WHITE)
        c=0;
      else c=1;
      putc(c,outfile);
      putc(c,outfile);
      putc(c,outfile);
    }
  fclose(outfile);
}

void printbmp(outfile) /* prints picture in BMP format */
FILE *outfile;
{
  int i,j,c,s0, s, W1;

  fprintf(outfile,"BM");

  W1 = (3*Width+3);
  W1 -= W1 % 4;
  s0 = (strlen(cmdLine)+strlen("Command line:\n\n")+3) & 0xffc;
  s = s0 + 54+W1*Height; /* file size */
  putc(s&255,outfile);
  putc((s>>8)&255,outfile);
  putc((s>>16)&255,outfile);
  putc(s>>24,outfile);

  putc(0,outfile);
  putc(0,outfile);
  putc(0,outfile);
  putc(0,outfile);

  putc(54,outfile); /* offset to data */
  putc(0,outfile);
  putc(0,outfile);
  putc(0,outfile);

  putc(40,outfile); /* size of infoheader */
  putc(0,outfile);
  putc(0,outfile);
  putc(0,outfile);

  putc(Width&255,outfile);
  putc((Width>>8)&255,outfile);
  putc((Width>>16)&255,outfile);
  putc(Width>>24,outfile);

  putc(Height&255,outfile);
  putc((Height>>8)&255,outfile);
  putc((Height>>16)&255,outfile);
  putc(Height>>24,outfile);

  putc(1,outfile);  /* no. of planes = 1 */
  putc(0,outfile);

  putc(24,outfile);  /* bpp */
  putc(0,outfile);

  putc(0,outfile); /* no compression */
  putc(0,outfile);
  putc(0,outfile);
  putc(0,outfile);

  putc(0,outfile); /* image size (unspecified) */
  putc(0,outfile);
  putc(0,outfile);
  putc(0,outfile);

  putc(0,outfile); /* h. pixels/m */
  putc(32,outfile);
  putc(0,outfile);
  putc(0,outfile);

  putc(0,outfile); /* v. pixels/m */
  putc(32,outfile);
  putc(0,outfile);
  putc(0,outfile);

  putc(0,outfile); /* colours used (unspecified) */
  putc(0,outfile);
  putc(0,outfile);
  putc(0,outfile);


  putc(0,outfile); /* important colours (all) */
  putc(0,outfile);
  putc(0,outfile);
  putc(0,outfile);

  if (doshade) {
    for (j=Height-1; j>=0; j--) {
      for (i=0; i<Width; i++) {
        s =shades[i][j];
        c = s*btable[col[i][j]]/150;
        if (c>255) c=255;
        putc(c,outfile);
        c = s*gtable[col[i][j]]/150;
        if (c>255) c=255;
        putc(c,outfile);
        c = s*rtable[col[i][j]]/150;
        if (c>255) c=255;
        putc(c,outfile);
      }
      for (i=3*Width; i<W1; i++) putc(0,outfile);
    }
  } else {
    for (j=Height-1; j>=0; j--) {
      for (i=0; i<Width; i++) {
        putc(btable[col[i][j]],outfile);
        putc(gtable[col[i][j]],outfile);
        putc(rtable[col[i][j]],outfile);
      }
      for (i=3*Width; i<W1; i++) putc(0,outfile);
    }
  }

  fprintf(outfile,"Command line:\n%s\n",cmdLine);

  fclose(outfile);
}

void printbmpBW(outfile) /* prints picture in b/w BMP format */
FILE *outfile;
{
  int i,j,c,s,s0, W1;

  fprintf(outfile,"BM");

  W1 = (Width+31);
  W1 -= W1 % 32;
  s0 = (strlen(cmdLine)+strlen("Command line:\n\n")+3) & 0xffc;
  s = s0 + 62+(W1*Height)/8; /* file size */
  putc(s&255,outfile);
  putc((s>>8)&255,outfile);
  putc((s>>16)&255,outfile);
  putc(s>>24,outfile);

  putc(0,outfile);
  putc(0,outfile);
  putc(0,outfile);
  putc(0,outfile);

  putc(62,outfile); /* offset to data */
  putc(0,outfile);
  putc(0,outfile);
  putc(0,outfile);

  putc(40,outfile); /* size of infoheader */
  putc(0,outfile);
  putc(0,outfile);
  putc(0,outfile);

  putc(Width&255,outfile);
  putc((Width>>8)&255,outfile);
  putc((Width>>16)&255,outfile);
  putc(Width>>24,outfile);

  putc(Height&255,outfile);
  putc((Height>>8)&255,outfile);
  putc((Height>>16)&255,outfile);
  putc(Height>>24,outfile);

  putc(1,outfile);  /* no. of planes = 1 */
  putc(0,outfile);

  putc(1,outfile);  /* bpp */
  putc(0,outfile);

  putc(0,outfile); /* no compression */
  putc(0,outfile);
  putc(0,outfile);
  putc(0,outfile);

  putc(0,outfile); /* image size (unspecified) */
  putc(0,outfile);
  putc(0,outfile);
  putc(0,outfile);

  putc(0,outfile); /* h. pixels/m */
  putc(32,outfile);
  putc(0,outfile);
  putc(0,outfile);

  putc(0,outfile); /* v. pixels/m */
  putc(32,outfile);
  putc(0,outfile);
  putc(0,outfile);

  putc(2,outfile); /* colours used */
  putc(0,outfile);
  putc(0,outfile);
  putc(0,outfile);


  putc(2,outfile); /* important colours (2) */
  putc(0,outfile);
  putc(0,outfile);
  putc(0,outfile);

  putc(0,outfile); /* colour 0 = black */
  putc(0,outfile);
  putc(0,outfile);
  putc(0,outfile);

  putc(255,outfile); /* colour 1 = white */
  putc(255,outfile);
  putc(255,outfile);
  putc(255,outfile);

  for (j=Height-1; j>=0; j--)
    for (i=0; i<W1; i+=8) {
      if (i<Width && col[i][j] >= WHITE)
        c=128;
      else c=0;
      if (i+1<Width && col[i+1][j] >= WHITE)
        c+=64;
      if (i+2<Width && col[i+2][j] >= WHITE)
        c+=32;
      if (i+3<Width && col[i+3][j] >= WHITE)
        c+=16;
      if (i+4<Width && col[i+4][j] >= WHITE)
        c+=8;
      if (i+5<Width && col[i+5][j] >= WHITE)
        c+=4;
      if (i+6<Width && col[i+6][j] >= WHITE)
        c+=2;
      if (i+7<Width && col[i+7][j] >= WHITE)
        c+=1;
      putc(c,outfile);
    }

  fprintf(outfile,"Command line:\n%s\n",cmdLine);

  fclose(outfile);
}

char *nletters(int n, int c)
{
  int i;
  static char buffer[8];

  buffer[n] = '\0';

  for (i = n-1; i >= 0; i--)
  {
    buffer[i] = letters[c & 0x001F];
    c >>= 5;
  }

  return buffer;
}

void printxpm(outfile) /* prints picture in XPM (X-windows pixel map) format */
FILE *outfile;
{
  int x,y,i,nbytes;

  x = nocols - 1;
  for (nbytes = 0; x != 0; nbytes++)
    x >>= 5;

  fprintf(outfile,"/* XPM */\n");
  fprintf(outfile,"/* Command line: */\n/* %s*/\n",cmdLine);
  fprintf(outfile,"static char *xpmdata[] = {\n");
  fprintf(outfile,"/* width height ncolors chars_per_pixel */\n");
  fprintf(outfile,"\"%d %d %d %d\",\n", Width, Height, nocols, nbytes);
  fprintf(outfile,"/* colors */\n");
  for (i = 0; i < nocols; i++)
    fprintf(outfile,"\"%s c #%2.2X%2.2X%2.2X\",\n", 
            nletters(nbytes, i), rtable[i], gtable[i], btable[i]);

  fprintf(outfile,"/* pixels */\n");
  for (y = 0 ; y < Height; y++) {
    fprintf(outfile,"\"");
    for (x = 0; x < Width; x++)
      fprintf(outfile, "%s", nletters(nbytes, col[x][y]));
    fprintf(outfile,"\",\n");
  }
  fprintf(outfile,"};\n");

  fclose(outfile);
}

void printxpmBW(outfile) /* prints picture in XPM (X-windows pixel map) format */
FILE *outfile;
{
  int x,y,nbytes;

  x = nocols - 1;
  nbytes = 1;

  fprintf(outfile,"/* XPM */\n");
  fprintf(outfile,"/* Command line: */\n/* %s*/\n",cmdLine);
  fprintf(outfile,"static char *xpmdata[] = {\n");
  fprintf(outfile,"/* width height ncolors chars_per_pixel */\n");
  fprintf(outfile,"\"%d %d %d %d\",\n", Width, Height, 2, nbytes);
  fprintf(outfile,"/* colors */\n");

  fprintf(outfile,"\". c #FFFFFF\",\n");
  fprintf(outfile,"\"X c #000000\",\n");

  fprintf(outfile,"/* pixels */\n");
  for (y = 0 ; y < Height; y++) {
    fprintf(outfile,"\"");
    for (x = 0; x < Width; x++)
      fprintf(outfile, "%s",
              (col[x][y] < WHITE)
              ? "X" : ".");
    fprintf(outfile,"\",\n");
  }
  fprintf(outfile,"};\n");

  fclose(outfile);
}

void printheights(outfile) /* prints heightfield */
FILE *outfile;
{
  int i,j;

  for (j=0; j<Height; j++) {
    for (i=0; i<Width; i++)
      fprintf(outfile,"%d ",heights[i][j]);
    putc('\n',outfile);
  }
  fclose(outfile);
}

void print_error()
{
  fprintf(stderr,"Usage: planet [options]\n\n");
  fprintf(stderr,"See manual for details\n\n");
  exit(0);
}

void reportError(char c) {
  fprintf(stderr,"Missing or bad argument to option -%c\n",c);
  print_error();
}
