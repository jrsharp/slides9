#ifndef SLIDES9_H
#define SLIDES9_H

#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <keyboard.h>
#include <thread.h>
#include <bio.h>
#include <ctype.h>

/* Maximum sizes */
#define MAXSLIDES 1000  /* Maximum number of slides */
#define MAXLINES 100    /* Maximum lines per slide */
#define MAXLINELEN 1024  /* Maximum length of a line */
#define MAXCOLS 10       /* Maximum columns in a table */
#define MAXNEST 8        /* Maximum nesting level for lists */
#define MAXCODEBLOCKLINES 100

/* Application modes */
typedef enum {
    NORMAL,
    OVERVIEW,
    HELP
} Mode;

/* Markdown element types */
typedef enum {
    TEXT,
    HEADER1,
    HEADER2,
    HEADER3,
    HEADER4,
    HEADER5,
    HEADER6,
    LISTITEM,
    CODEBLOCK,
    BLOCKQUOTE,
    HORIZONTALRULE,
    EMPHASIS,
    STRONG,
    TABLE_ROW,
    TABLE_SEPARATOR,
    IMAGE
} ElementType;

/* Text formatting flags (can be combined) */
enum {
    FMT_NONE = 0,
    FMT_BOLD = 1<<0,
    FMT_ITALIC = 1<<1,
    FMT_STRIKE = 1<<2,
    FMT_CODE = 1<<3,
    FMT_LINK = 1<<4
};

/* Structure for a table cell */
typedef struct {
    char *text;
    int width;
    int align;  /* -1 for left, 0 for center, 1 for right */
} TableCell;

/* Structure for a table row */
typedef struct {
    TableCell cells[MAXCOLS];
    int ncells;
} TableRow;

/* Structure for a formatted text segment */
typedef struct TextSegment {
    char text[MAXLINELEN];
    int format;         /* Combination of formatting flags */
    struct TextSegment *next;
} TextSegment;

/* Structure for an image */
typedef struct {
    char path[MAXLINELEN];
    char alt[MAXLINELEN];
    Image *img;
} ImageData;

/* Structure for a line of text with formatting */
typedef struct {
    TextSegment *segments;  /* Linked list of formatted segments */
    char text[MAXLINELEN]; /* Original text (for parsing) */
    ElementType type;       /* Markdown element type */
    int indent;             /* Indentation level */
    int list_level;         /* Nesting level for lists */
    int in_codeblock;       /* Whether this line is part of a code block */
    char lang[32];          /* Language for code blocks */
    TableRow table;         /* Table data if this is a table row */
    struct CodeBlock *codeblock;
    ImageData *image;       /* Image data if this is an image */
} Line;

/* Structure for a code block */
typedef struct CodeBlock {
    char lang[32];
    char **lines;  /* Array of strings */
    int nlines;
    int capacity;
} CodeBlock;

/* Structure for a single slide */
typedef struct {
    Line lines[MAXLINES];
    int nlines;
    CodeBlock *codeblocks;  /* Array of code blocks in this slide */
    int ncodeblocks;
    CodeBlock *current_block;
} Slide;

/* Global variables */
extern Mode mode;
extern Slide slides[MAXSLIDES];
extern int nslides;
extern int current_slide;
extern Image *screen;
extern Font *font;
extern Font *bold;      /* Bold font for headers */
extern Font *italic;    /* Italic font for emphasis */
extern Font *h1font;    /* Larger font for H1 */
extern Font *h2font;    /* Larger font for H2 */
extern Font *h3font;    /* Larger font for H3 */
extern Font *h4font;    /* Larger font for H4 */
extern Font *h5font;    /* Bold font for H5 */
extern Font *h6font;    /* Italic font for H6 */
extern struct Display *display;
extern Rectangle viewrect;     /* Rectangle for slide content */
extern Point margin;           /* Margin for text */
extern int line_height;        /* Height of a line of text */

/* Colors */
extern Image *back;    /* Background - dark */
extern Image *text;    /* Text - light */
extern Image *accent;  /* Accent - highlight */
extern Image *dim;     /* Dimmed text */
extern Image *quote;   /* Blockquote background */
extern Image *table;   /* Table border color */
extern Image *lightgrey;  /* Light grey for code blocks */
extern Image *h1color;    /* Header 1 color */
extern Image *h2color;    /* Header 2 color */
extern Image *h3color;    /* Header 3 color */
extern Image *h4color;    /* Header 4 color */
extern Image *h5color;    /* Header 5 color */
extern Image *h6color;    /* Header 6 color */

/* Syntax highlighting colors */
extern Image *keyword_color;   /* Language keywords */
extern Image *string_color;    /* String literals */
extern Image *number_color;    /* Numeric literals */
extern Image *comment_color;   /* Comments */
extern Image *type_color;      /* Types and classes */
extern Image *function_color;  /* Function names */

/* Function prototypes */
void setup(void);
int loadslides(char *filename);
void drawslide(int n);
void drawprogress(void);
void usage(void);
void handlekey(Rune key);
void resdraw(void);
void eresized(int new);
int parseline(char *line, Line *out);
void switchmode(int newmode);
void parseformatting(Line *line);
void freeline(Line *line);
void drawtext(char *text, Point p, Image *col, Font *f, int maxwidth);
TextSegment *newsegment(char *text, int format);
int istabledelimiter(char *line);
void parsetable(char *line, TableRow *row);
void drawtable(TableRow *row, Point *p);
void startcodeblock(Slide *s, char *lang);
void addtocodeblock(Slide *s, char *line);
void endcodeblock(Slide *s);
void freecodeblock(CodeBlock *cb);
void processcodeblock(char *line, Slide *current);
void drawcodeblock(CodeBlock *cb, Point *p);
ImageData *parseimage(char *line);
void drawimage(ImageData *img, Point *p);
void freeimagedata(ImageData *img);

#endif 