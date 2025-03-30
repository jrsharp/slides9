#include "slides9.h"

/* Global variables */
Mode mode = NORMAL;
Slide slides[MAXSLIDES];  /* Changed from pointer to array */
int nslides = 0;
int current_slide = 0;
int is_fullscreen = 0;  /* Track window state */
Rectangle saved_rect;    /* Save window dimensions for toggling */
struct Image *screen;
struct Font *font;
struct Font *bold;
struct Font *italic;
struct Font *h1font;    /* Larger font for H1 */
struct Font *h2font;    /* Larger font for H2 */
struct Font *h3font;    /* Larger font for H3 */
struct Font *h4font;    /* Larger font for H4 */
struct Font *h5font;    /* Bold font for H5 */
struct Font *h6font;    /* Italic font for H6 */
struct Display *display;
Rectangle viewrect;
Point margin;
int line_height;
char *argv0;

/* Colors */
Image *back;    /* Background - dark */
Image *text;    /* Text - light */
Image *accent;  /* Accent - highlight */
Image *dim;     /* Dimmed text */
Image *quote;   /* Blockquote background */
Image *table;   /* Table border color */
Image *lightgrey;  /* Light grey for code blocks */
Image *h1color;
Image *h2color;
Image *h3color;
Image *h4color;
Image *h5color;
Image *h6color;

/* Global variables for syntax highlighting */
Image *keyword_color;
Image *string_color;
Image *number_color;
Image *comment_color;
Image *type_color;
Image *function_color;

void
initcolors(void)
{
    /* Base colors */
    back = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0xFFFFFFFF);  /* Pure white background */
    text = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x000000FF);  /* Pure black text */
    
    /* Headers and accents */
    accent = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x0066CCFF); /* Bright blue */
    dim = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x555555FF);   /* Darker grey for better contrast */
    
    /* Special elements */
    quote = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0xEEEEEEFF);  /* Light grey for quotes */
    table = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x888888FF);  /* Darker grey for tables */
    lightgrey = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0xF0F0F0FF); /* Very light grey for code */
    
    /* Header colors - higher contrast and more saturated */
    h1color = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0xCC0000FF); /* Dark red */
    h2color = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x0055AAFF); /* Dark blue */
    h3color = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x007700FF); /* Dark green */
    h4color = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x660099FF); /* Dark purple */
    h5color = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x993300FF); /* Dark orange */
    h6color = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x006666FF); /* Dark teal */
    
    /* Syntax highlighting colors - more saturated for better contrast */
    keyword_color = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0xAA0000FF);  /* Dark red */
    string_color = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x007700FF);   /* Dark green */
    number_color = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0xCC5500FF);   /* Orange */
    comment_color = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x555555FF);  /* Grey */
    type_color = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x0055AAFF);     /* Blue */
    function_color = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x660099FF);  /* Purple */
    
    if(back == nil || text == nil || accent == nil || dim == nil || 
       quote == nil || table == nil || lightgrey == nil ||
       h1color == nil || h2color == nil || h3color == nil ||
       h4color == nil || h5color == nil || h6color == nil ||
       keyword_color == nil || string_color == nil || number_color == nil ||
       comment_color == nil || type_color == nil || function_color == nil)
        sysfatal("allocimage failed");
}

void
setupdraw(void)
{
    char *fontpath, *fontlargepath, *fontsmallpath;
    
    if(display == nil)
        sysfatal("display not initialized");
    
    screen = display->image;
    if(screen == nil)
        sysfatal("screen not initialized");
    
    /* Use system default font as base font */
    font = display->defaultfont;
    if(font == nil)
        sysfatal("cannot load default font");
    
    /* Check environment for font settings */
    fontpath = getenv("font");
    fontlargepath = getenv("fontlarge");
    fontsmallpath = getenv("fontsmall");
    
    /* Load custom font if specified */
    if(fontpath != nil) {
        Font *customfont = openfont(display, fontpath);
        if(customfont != nil)
            font = customfont;
        free(fontpath);
    }
    
    /* Load custom large font if specified */
    if(fontlargepath != nil) {
        h1font = h2font = openfont(display, fontlargepath);
        free(fontlargepath);
    } else {
        /* Try to load larger fonts for headers - try both pelm and lucsans */
        h1font = openfont(display, "/lib/font/bit/pelm/unicode.24.font");
        if(h1font == nil)
            h1font = openfont(display, "/lib/font/bit/lucsans/unicode.24.font");
        
        h2font = openfont(display, "/lib/font/bit/pelm/unicode.20.font");
        if(h2font == nil)
            h2font = openfont(display, "/lib/font/bit/lucsans/unicode.20.font");
        
        /* Fallback to default font if larger sizes not available */
        if(h1font == nil)
            h1font = font;
        if(h2font == nil)
            h2font = font;
    }
    
    /* Load custom small font if specified */
    if(fontsmallpath != nil) {
        h6font = openfont(display, fontsmallpath);
        free(fontsmallpath);
    }
    
    /* Try to load medium fonts for h3 and h4 */
    h3font = openfont(display, "/lib/font/bit/pelm/unicode.16.font");
    if(h3font == nil)
        h3font = openfont(display, "/lib/font/bit/lucsans/unicode.16.font");
    
    h4font = openfont(display, "/lib/font/bit/pelm/unicode.14.font");
    if(h4font == nil)
        h4font = openfont(display, "/lib/font/bit/lucsans/unicode.14.font");
    
    /* Fallback to default font if not available */
    if(h3font == nil)
        h3font = font;
    if(h4font == nil)
        h4font = font;
    
    /* Load bold and italic variants of the default font size */
    h5font = bold = openfont(display, "/lib/font/bit/lucsans/typebold.14.font");
    if(h5font == nil)
        h5font = bold = font;
    
    if(h6font == nil) {
        h6font = italic = openfont(display, "/lib/font/bit/lucsans/typeitalic.14.font");
        if(h6font == nil)
            h6font = italic = font;
    }
    
    /* Adjust line height based on font size */
    line_height = font->height + 4;  /* More spacing between lines */
    margin = Pt(40, 40);  /* Set default margins */
    
    /* Initialize colors */
    initcolors();
}

TextSegment*
newsegment(char *text, int format)
{
    TextSegment *seg = malloc(sizeof(TextSegment));
    if(seg == nil)
        sysfatal("malloc failed");
    strncpy(seg->text, text, MAXLINELEN-1);
    seg->text[MAXLINELEN-1] = '\0';
    seg->format = format;
    seg->next = nil;
    return seg;
}

void
freeimagedata(ImageData *img)
{
    if(img == nil)
        return;
    
    if(img->img != nil) {
        /* Use Plan 9's freeimage function */
        freeimage(img->img);
        img->img = nil;
    }
    
    free(img);
}

void
freeline(Line *line)
{
    TextSegment *seg, *next;
    int i;
    
    /* Free text segments */
    seg = line->segments;
    while(seg != nil) {
        next = seg->next;
        free(seg);
        seg = next;
    }
    
    /* Free table cell text if allocated */
    for(i = 0; i < line->table.ncells; i++) {
        if(line->table.cells[i].text != nil)
            free(line->table.cells[i].text);
    }
    
    /* Free image if present */
    if(line->image != nil)
        freeimagedata(line->image);
}

void
parseformatting(Line *line)
{
    char *p = line->text;
    char *start = p;
    char *end;
    int in_emphasis = 0;
    int in_strong = 0;
    int in_code = 0;
    
    while(*p) {
        if(*p == '*' || *p == '_') {
            if(*(p+1) == '*' || *(p+1) == '_') {
                /* Strong emphasis */
                if(!in_strong) {
                    start = p + 2;
                    in_strong = 1;
                } else {
                    end = p;
                    *end = '\0';
                    line->type = STRONG;
                    strncpy(line->text, start, MAXLINELEN-1);
                    in_strong = 0;
                }
                p++;
            } else {
                /* Regular emphasis */
                if(!in_emphasis) {
                    start = p + 1;
                    in_emphasis = 1;
                } else {
                    end = p;
                    *end = '\0';
                    line->type = EMPHASIS;
                    strncpy(line->text, start, MAXLINELEN-1);
                    in_emphasis = 0;
                }
            }
        } else if(*p == '`') {
            /* Inline code */
            if(!in_code) {
                start = p + 1;
                in_code = 1;
            } else {
                end = p;
                *end = '\0';
                line->type = CODEBLOCK;
                strncpy(line->text, start, MAXLINELEN-1);
                in_code = 0;
            }
        }
        p++;
    }
    
    /* Ensure null termination */
    line->text[MAXLINELEN-1] = '\0';
}

int
parseline(char *line, Line *out)
{
    char *p = line;
    int indent = 0;
    
    /* Initialize line */
    memset(out, 0, sizeof(Line));
    strncpy(out->text, line, MAXLINELEN-1);
    out->text[MAXLINELEN-1] = '\0';
    
    /* Skip leading whitespace and count indentation */
    while(*p == ' ' || *p == '\t') {
        indent++;
        p++;
    }
    
    /* Store the indentation */
    out->indent = indent;
    
    /* Calculate list nesting level (2 spaces per level) */
    out->list_level = indent / 2;
    if(out->list_level >= MAXNEST)
        out->list_level = MAXNEST - 1;

    /* Check for image syntax ![alt](path) */
    if(p[0] == '!' && p[1] == '[') {
        out->type = IMAGE;
        out->image = parseimage(p);
        return 1;
    }
    
    /* Check for table rows */
    if(strchr(line, '|') != nil) {
        if(istabledelimiter(line)) {
            out->type = TABLE_SEPARATOR;
        } else {
            out->type = TABLE_ROW;
        }
        parsetable(line, &out->table);
        return 1;
    }
    
    /* Check for markdown block elements */
    if(strncmp(p, "# ", 2) == 0) {
        out->type = HEADER1;
        strncpy(out->text, p + 2, MAXLINELEN-1);
    } else if(strncmp(p, "## ", 3) == 0) {
        out->type = HEADER2;
        strncpy(out->text, p + 3, MAXLINELEN-1);
    } else if(strncmp(p, "### ", 4) == 0) {
        out->type = HEADER3;
        strncpy(out->text, p + 4, MAXLINELEN-1);
    } else if(strncmp(p, "#### ", 5) == 0) {
        out->type = HEADER4;
        strncpy(out->text, p + 5, MAXLINELEN-1);
    } else if(strncmp(p, "##### ", 6) == 0) {
        out->type = HEADER5;
        strncpy(out->text, p + 6, MAXLINELEN-1);
    } else if(strncmp(p, "###### ", 7) == 0) {
        out->type = HEADER6;
        strncpy(out->text, p + 7, MAXLINELEN-1);
    } else if(strncmp(p, "* ", 2) == 0 || strncmp(p, "- ", 2) == 0) {
        out->type = LISTITEM;
        strncpy(out->text, p + 2, MAXLINELEN-1);
    } else if(strncmp(p, "> ", 2) == 0) {
        out->type = BLOCKQUOTE;
        strncpy(out->text, p + 2, MAXLINELEN-1);
    } else if(strncmp(p, "---", 3) == 0 || strncmp(p, "***", 3) == 0) {
        out->type = HORIZONTALRULE;
        return 1;
    } else {
        out->type = TEXT;
        strncpy(out->text, p, MAXLINELEN-1);
    }
    
    out->text[MAXLINELEN-1] = '\0';
    
    /* Parse inline formatting */
    parseformatting(out);
    
    return 1;
}

void
drawtext(char *text, Point p, Image *col, Font *f, int maxwidth)
{
    char line[MAXLINELEN];
    char word[MAXLINELEN];
    int start_y = p.y;
    char *t = text;
    int pos = 0;
    int word_pos = 0;
    int line_width = 0;
    Rectangle bounds = screen->r;
    int lines_drawn = 0;
    
    /* If text fits on one line, just draw it */
    if(stringwidth(f, text) <= maxwidth) {
        string(screen, p, col, ZP, f, text);
        p.y += f->height;
        return;
    }
    
    /* Start wrapping text by words */
    memset(line, 0, sizeof(line));
    memset(word, 0, sizeof(word));
    
    while(*t) {
        /* Handle whitespace */
        if(*t == ' ' || *t == '\t' || *t == '\n') {
            /* End current word */
            if(word_pos > 0) {
                word[word_pos] = '\0';
                
                /* Check if adding this word exceeds line width */
                int word_width = stringwidth(f, word);
                
                /* If this is a single word that's too long, we need to draw it anyway */
                if(pos == 0 && word_width > maxwidth) {
                    strcpy(line, word);
                    pos = word_pos;
                    line[pos] = '\0';
                    
                    if(p.y + f->height <= bounds.max.y - margin.y) {
                        string(screen, p, col, ZP, f, line);
                        p.y += f->height;
                        lines_drawn++;
                    }
                    
                    pos = 0;
                    line_width = 0;
                    memset(line, 0, sizeof(line));
                }
                /* Check if adding this word would exceed line width */
                else if(pos > 0 && line_width + word_width + stringwidth(f, " ") > maxwidth) {
                    /* Line would be too long, end current line */
                    line[pos] = '\0';
                    if(p.y + f->height <= bounds.max.y - margin.y) {
                        string(screen, p, col, ZP, f, line);
                        p.y += f->height;
                        lines_drawn++;
                    }
                    
                    /* Start a new line with this word */
                    pos = 0;
                    line_width = 0;
                    memset(line, 0, sizeof(line));
                    
                    strcpy(line, word);
                    pos = word_pos;
                    line_width = word_width;
                }
                else {
                    /* Add word to line */
                    if(pos > 0 && pos < MAXLINELEN-1) {
                        line[pos++] = ' ';
                        line_width += stringwidth(f, " ");
                    }
                    
                    strncpy(line + pos, word, MAXLINELEN - pos - 1);
                    pos += word_pos;
                    line_width += word_width;
                }
                
                /* Reset word buffer */
                word_pos = 0;
                memset(word, 0, sizeof(word));
            }
            
            /* Skip consecutive whitespace */
            while(*t == ' ' || *t == '\t' || *t == '\n') 
                t++;
            continue;
        }
        
        /* Add character to current word */
        if(word_pos < MAXLINELEN-1) {
            word[word_pos++] = *t;
        }
        t++;
    }
    
    /* Add final word if any */
    if(word_pos > 0) {
        word[word_pos] = '\0';
        
        /* Check if adding this word exceeds line width */
        int word_width = stringwidth(f, word);
        
        /* If this is a single word that's too long, we need to draw it anyway */
        if(pos == 0 && word_width > maxwidth) {
            strcpy(line, word);
            pos = word_pos;
        }
        /* Check if adding this word would exceed line width */
        else if(pos > 0 && line_width + word_width + stringwidth(f, " ") > maxwidth) {
            /* Line would be too long, end current line */
            line[pos] = '\0';
            if(p.y + f->height <= bounds.max.y - margin.y) {
                string(screen, p, col, ZP, f, line);
                p.y += f->height;
                lines_drawn++;
            }
            
            /* Start a new line with this word */
            pos = 0;
            line_width = 0;
            memset(line, 0, sizeof(line));
            
            strcpy(line, word);
            pos = word_pos;
        }
        else {
            /* Add word to line */
            if(pos > 0 && pos < MAXLINELEN-1) {
                line[pos++] = ' ';
                line_width += stringwidth(f, " ");
            }
            
            strncpy(line + pos, word, MAXLINELEN - pos - 1);
            pos += word_pos;
        }
    }
    
    /* Draw final line if any */
    if(pos > 0) {
        line[pos] = '\0';
        if(p.y + f->height <= bounds.max.y - margin.y) {
            string(screen, p, col, ZP, f, line);
            p.y += f->height;
            lines_drawn++;
        }
    }
    
    /* If no lines were drawn, we still need to advance y position */
    if(lines_drawn == 0 && p.y == start_y) {
        p.y += f->height;
    }
}

void
drawtable(TableRow *row, Point *p)
{
    int i;
    int x = p->x;
    Rectangle bounds = screen->r;
    int maxwidth = bounds.max.x - bounds.min.x - 2*margin.x;  /* Available width */
    int colwidth = maxwidth / (row->ncells > 0 ? row->ncells : 1);
    int cellpadding = 10;
    
    /* Skip if we're outside window bounds */
    if(p->y < bounds.min.y || p->y >= bounds.max.y - margin.y)
        return;
    
    /* Draw cells */
    for(i = 0; i < row->ncells; i++) {
        TableCell *cell = &row->cells[i];
        Point cellp = *p;
        int textwidth;
        
        /* Stop if we exceed window bounds */
        if(x + colwidth > bounds.max.x - margin.x)
            break;
        
        /* Calculate cell rectangle */
        Rectangle r = Rect(x, p->y, x + colwidth, p->y + font->height + 2*cellpadding);
        
        /* Skip if cell is outside window bounds */
        if(r.max.y > bounds.max.y - margin.y)
            break;
        
        /* Draw cell background and borders */
        draw(screen, r, display->white, nil, ZP);
        
        /* Only draw borders if they're within window bounds */
        if(r.min.y >= bounds.min.y && r.max.y <= bounds.max.y - margin.y) {
            line(screen, Pt(r.min.x, r.min.y), Pt(r.max.x, r.min.y), 0, 0, 1, display->black, ZP);
            line(screen, Pt(r.min.x, r.max.y), Pt(r.max.x, r.max.y), 0, 0, 1, display->black, ZP);
            line(screen, Pt(r.min.x, r.min.y), Pt(r.min.x, r.max.y), 0, 0, 1, display->black, ZP);
            line(screen, Pt(r.max.x, r.min.y), Pt(r.max.x, r.max.y), 0, 0, 1, display->black, ZP);
        }
        
        /* Calculate text position based on alignment */
        textwidth = stringwidth(font, cell->text);
        cellp.x = x + cellpadding;
        cellp.y = p->y + cellpadding;
        
        if(cell->align == 0)  /* Center */
            cellp.x = x + (colwidth - textwidth)/2;
        else if(cell->align == 1)  /* Right */
            cellp.x = x + colwidth - textwidth - cellpadding;
        
        /* Draw cell text if it fits within window bounds */
        if(cellp.y + font->height <= bounds.max.y - margin.y && 
           cellp.x + textwidth <= bounds.max.x - margin.x) {
            string(screen, cellp, display->black, ZP, font, cell->text);
        }
        
        x += colwidth;
    }
    
    /* Update drawing position */
    p->y += font->height + 2*cellpadding;
}

void
drawslide(int n)
{
    Point p;
    Line *line;
    int i;
    Rectangle bounds;
    
    if(n < 0 || n >= nslides)
        return;
    
    /* Calculate content bounds based on window size */
    bounds = screen->r;
    
    /* Clear screen with white background */
    draw(screen, bounds, display->white, nil, ZP);
    
    /* Draw slide content */
    p = addpt(bounds.min, margin);  /* Start at window bounds + margin */
    
    /* Ensure we don't start drawing outside window bounds */
    if(p.y > bounds.max.y - margin.y)
        return;
    
    for(i = 0; i < slides[n].nlines; i++) {
        line = &slides[n].lines[i];
        
        /* Skip if we're past the bottom of the window */
        if(p.y > bounds.max.y - margin.y)
            break;
            
        /* Apply indentation, but don't exceed window bounds */
        p.x = bounds.min.x + margin.x + line->indent * 10;
        if(p.x > bounds.max.x - margin.x)
            p.x = bounds.max.x - margin.x;
        
        /* Calculate available width for this line */
        int available_width = bounds.max.x - margin.x - p.x;
        if(available_width < 0)
            available_width = 0;
        
        switch(line->type) {
        case HEADER1:
            if(p.y + h1font->height <= bounds.max.y - margin.y) {
                /* More space before H1 */
                p.y += 20;
                
                /* Draw as a single operation */
                if(p.y + h1font->height <= bounds.max.y - margin.y) {
                    string(screen, p, h1color, ZP, h1font, line->text);
                    p.y += h1font->height + 10; /* Add space after header */
                }
            } else {
                p.y = bounds.max.y;  /* Force exit from loop */
            }
            break;
            
        case HEADER2:
            if(p.y + h2font->height <= bounds.max.y - margin.y) {
                /* Space before H2 */
                p.y += 15;
                
                /* Draw as a single operation */
                if(p.y + h2font->height <= bounds.max.y - margin.y) {
                    string(screen, p, h2color, ZP, h2font, line->text);
                    p.y += h2font->height + 8; /* Add space after header */
                }
            } else {
                p.y = bounds.max.y;
            }
            break;
            
        case HEADER3:
            if(p.y + h3font->height <= bounds.max.y - margin.y) {
                /* Space before H3 */
                p.y += 10;
                
                /* Draw as a single operation */
                if(p.y + h3font->height <= bounds.max.y - margin.y) {
                    string(screen, p, h3color, ZP, h3font, line->text);
                    p.y += h3font->height + 6; /* Add space after header */
                }
            } else {
                p.y = bounds.max.y;
            }
            break;
            
        case HEADER4:
            if(p.y + h4font->height <= bounds.max.y - margin.y) {
                /* Small space before H4 */
                p.y += 5;
                
                /* Draw as a single operation */
                if(p.y + h4font->height <= bounds.max.y - margin.y) {
                    string(screen, p, h4color, ZP, h4font, line->text);
                    p.y += h4font->height + 4; /* Add space after header */
                }
            } else {
                p.y = bounds.max.y;
            }
            break;
            
        case HEADER5:
            if(p.y + h5font->height <= bounds.max.y - margin.y) {
                /* Draw as a single operation */
                string(screen, p, h5color, ZP, h5font, line->text);
                p.y += h5font->height + 2; /* Add space after header */
            } else {
                p.y = bounds.max.y;
            }
            break;
            
        case HEADER6:
            if(p.y + h6font->height <= bounds.max.y - margin.y) {
                /* Draw as a single operation */
                string(screen, p, h6color, ZP, h6font, line->text);
                p.y += h6font->height + 2; /* Add space after header */
            } else {
                p.y = bounds.max.y;
            }
            break;
            
        case LISTITEM:
            if(p.y + font->height <= bounds.max.y - margin.y) {
                string(screen, p, display->black, ZP, font, "•");
                p.x += 20;
                
                /* Recalculate available width after bullet point */
                available_width = bounds.max.x - margin.x - p.x;
                if(available_width < 0)
                    available_width = 0;
                    
                if(p.x < bounds.max.x - margin.x) {
                    /* Use drawtext for wrapping */
                    Point listP = p;
                    drawtext(line->text, listP, display->black, font, available_width);
                    /* We need to manually advance p.y here since drawtext modifies a copy */
                    int textWidth = stringwidth(font, line->text);
                    if(textWidth <= available_width) {
                        p.y += font->height; /* Single line */
                    } else {
                        /* Estimate wrapped lines - this is imprecise but better than nothing */
                        int lines = (textWidth / available_width) + 1;
                        p.y += font->height * lines;
                    }
                }
                    
                /* Reset p.x for the next line */
                p.x = bounds.min.x + margin.x + line->indent * 10;
            } else {
                p.y = bounds.max.y;
            }
            break;
            
        case BLOCKQUOTE:
            if(p.y + font->height <= bounds.max.y - margin.y) {
                Rectangle quote_bar = Rect(bounds.min.x + margin.x, p.y, 
                                           bounds.min.x + margin.x + 3, 
                                           p.y + font->height * 2);
                                           
                /* Draw quote bar */
                if(quote_bar.max.x <= bounds.max.x - margin.x) {
                    draw(screen, quote_bar, display->black, nil, ZP);
                }
                
                /* Move text right of the quote bar */
                p.x += 10;
                available_width -= 10;
                
                if(available_width > 0) {
                    Point quoteP = p;
                    drawtext(line->text, quoteP, display->black, italic, available_width);
                    /* We need to manually advance p.y here since drawtext modifies a copy */
                    int textWidth = stringwidth(font, line->text);
                    if(textWidth <= available_width) {
                        p.y += font->height; /* Single line */
                    } else {
                        /* Estimate wrapped lines - this is imprecise but better than nothing */
                        int lines = (textWidth / available_width) + 1;
                        p.y += font->height * lines;
                    }
                }
                    
                /* Reset p.x for the next line */
                p.x = bounds.min.x + margin.x + line->indent * 10;
            } else {
                p.y = bounds.max.y;
            }
            break;
            
        case HORIZONTALRULE:
            if(p.y + font->height/2 <= bounds.max.y - margin.y) {
                Rectangle rule = Rect(bounds.min.x + margin.x, p.y + font->height/2 - 1,
                                   bounds.max.x - margin.x, p.y + font->height/2 + 1);
                draw(screen, rule, display->black, nil, ZP);
                p.y += font->height;
            } else {
                p.y = bounds.max.y;
            }
            break;
            
        case TABLE_ROW:
        case TABLE_SEPARATOR:
            if(p.y + font->height + 20 <= bounds.max.y - margin.y)
                drawtable(&line->table, &p);
            else
                p.y = bounds.max.y;
            break;
            
        case CODEBLOCK:
            if(p.y + font->height <= bounds.max.y - margin.y)
                drawcodeblock(line->codeblock, &p);
            else
                p.y = bounds.max.y;
            break;
            
        case IMAGE:
            if(line->image != nil && p.y < bounds.max.y - margin.y) {
                drawimage(line->image, &p);
            } else {
                p.y = bounds.max.y;
            }
            break;
            
        default:
            if(p.y + font->height <= bounds.max.y - margin.y) {
                Point textP = p;
                drawtext(line->text, textP, display->black, font, available_width);
                /* We need to manually advance p.y here since drawtext modifies a copy */
                int textWidth = stringwidth(font, line->text);
                if(textWidth <= available_width) {
                    p.y += font->height; /* Single line */
                } else {
                    /* Estimate wrapped lines - this is imprecise but better than nothing */
                    int lines = (textWidth / available_width) + 1;
                    p.y += font->height * lines;
                }
            } else {
                p.y = bounds.max.y;
            }
            break;
        }
        
        /* Reset x position for next line */
        p.x = bounds.min.x + margin.x;
        
        /* Break if we've hit the bottom of the window */
        if(p.y >= bounds.max.y - margin.y)
            break;
    }
    
    /* Draw progress bar */
    drawprogress();
    
    /* Flush screen */
    flushimage(display, 1);
}

void
drawprogress(void)
{
    Rectangle r, bounds;
    int width, pos;
    
    bounds = screen->r;
    
    /* Create progress bar rectangle at bottom of window */
    r = Rect(bounds.min.x + margin.x, bounds.max.y - margin.y/2,
             bounds.max.x - margin.x, bounds.max.y - margin.y/4);
    
    /* Draw background */
    draw(screen, r, dim, nil, ZP);
    
    /* Calculate and draw progress */
    width = r.max.x - r.min.x;
    pos = (width * (current_slide + 1)) / nslides;
    r.max.x = r.min.x + pos;
    draw(screen, r, accent, nil, ZP);
}

void
togglewindow(void)
{
    char cmd[128];
    int fd;
    
    is_fullscreen = !is_fullscreen;
    
    if(is_fullscreen) {
        /* Save current window dimensions before going fullscreen */
        saved_rect = screen->r;
        snprint(cmd, sizeof cmd, "window/resize -r 0 0 9999 9999");
    } else {
        /* Return to previous windowed dimensions */
        snprint(cmd, sizeof cmd, "window/resize -r %d %d %d %d",
                saved_rect.min.x, saved_rect.min.y, 
                saved_rect.max.x, saved_rect.max.y);
    }
    
    fd = open("/dev/wctl", OWRITE);
    if(fd >= 0) {
        write(fd, cmd, strlen(cmd));
        close(fd);
    }
    
    /* Redraw after resize */
    if(getwindow(display, Refnone) < 0)
        sysfatal("getwindow: %r");
    drawslide(current_slide);
}

void
handlekey(Rune key)
{
    switch(key) {
    case Kesc:
        if(mode != NORMAL)
            switchmode(NORMAL);
        break;
    case 'q':
        exits(nil);
        break;
    case 'f':  /* Toggle fullscreen */
        togglewindow();
        break;
    case 'h':
    case Kleft:
        if(current_slide > 0) {
            current_slide--;
            drawslide(current_slide);
        }
        break;
    case 'l':
    case Kright:
        if(current_slide < nslides - 1) {
            current_slide++;
            drawslide(current_slide);
        }
        break;
    case 'g':  /* Go to first slide */
        current_slide = 0;
        drawslide(current_slide);
        break;
    case 'G':  /* Go to last slide */
        current_slide = nslides - 1;
        drawslide(current_slide);
        break;
    case 'o':  /* Toggle overview mode */
        switchmode(mode == OVERVIEW ? NORMAL : OVERVIEW);
        break;
    case '?':  /* Toggle help mode */
        switchmode(mode == HELP ? NORMAL : HELP);
        break;
    }
}

void
switchmode(int newmode)
{
    mode = newmode;
    drawslide(current_slide);
}

void
usage(void)
{
    fprint(2, "usage: %s file\n", argv0);
    exits("usage");
}

void
resdraw(void)
{
    if(getwindow(display, Refnone) < 0)
        sysfatal("getwindow: %r");
    drawslide(current_slide);
}

void
eresized(int new)
{
    if(new && getwindow(display, Refnone) < 0)
        sysfatal("can't reattach to window");
    drawslide(current_slide);
}

int
loadslides(char *filename)
{
    Biobuf *bp;
    char *line;
    Slide *current;
    int len;
    
    bp = Bopen(filename, OREAD);
    if(bp == nil)
        return -1;
    
    nslides = 0;
    current = &slides[nslides++];
    memset(current, 0, sizeof(Slide));
    
    while((line = Brdline(bp, '\n')) != nil) {
        len = Blinelen(bp);
        line[len-1] = '\0';  /* Remove newline */
        
        /* Skip empty lines at start of file */
        if(current->nlines == 0 && line[0] == '\0')
            continue;
        
        /* Check for slide separator */
        if(strcmp(line, "---") == 0) {
            if(current->nlines > 0 && nslides < MAXSLIDES) {
                current = &slides[nslides++];
                memset(current, 0, sizeof(Slide));
            }
            continue;
        }
        
        /* Process code blocks */
        if(strncmp(line, "```", 3) == 0 || current->current_block != nil) {
            processcodeblock(line, current);
            continue;
        }
        
        /* Skip empty lines between slides */
        if(line[0] == '\0' && current->nlines == 0)
            continue;
        
        /* Add line to current slide */
        if(current->nlines < MAXLINES) {
            Line *l = &current->lines[current->nlines++];
            memset(l, 0, sizeof(Line));
            parseline(line, l);
        }
    }
    
    Bterm(bp);
    return nslides;
}

void
cleanup(void)
{
    int i, j;
    Line *line;
    
    /* Free code blocks in all slides */
    for(i = 0; i < nslides; i++) {
        for(j = 0; j < slides[i].nlines; j++) {
            line = &slides[i].lines[j];
            if(line->type == CODEBLOCK && line->codeblock != nil) {
                freecodeblock(line->codeblock);
                line->codeblock = nil;
            }
        }
        /* Free any unfinished code block */
        if(slides[i].current_block != nil) {
            freecodeblock(slides[i].current_block);
            slides[i].current_block = nil;
        }
    }
}

int
istabledelimiter(char *line)
{
    char *p = line;
    int has_pipe = 0;
    
    while(*p) {
        if(*p == '|')
            has_pipe = 1;
        else if(*p != '-' && *p != ':' && *p != ' ' && *p != '\t')
            return 0;
        p++;
    }
    
    return has_pipe;
}

void
parsetable(char *line, TableRow *row)
{
    char *p = line;
    char *start = p;
    int col = 0;
    
    /* Initialize row */
    memset(row, 0, sizeof(TableRow));
    
    /* Skip leading pipe */
    if(*p == '|')
        p++;
    
    while(*p && col < MAXCOLS) {
        if(*p == '|') {
            /* End of cell */
            int len = p - start;
            char *cell = malloc(len + 1);
            if(cell == nil)
                sysfatal("malloc failed");
            
            /* Copy and trim cell content */
            strncpy(cell, start, len);
            cell[len] = '\0';
            
            /* Trim whitespace */
            while(len > 0 && isspace(cell[len-1]))
                cell[--len] = '\0';
            while(*cell && isspace(*cell)) {
                cell++;
                len--;
            }
            
            row->cells[col].text = cell;
            row->cells[col].width = len;
            
            /* Default to left alignment */
            row->cells[col].align = -1;
            
            col++;
            start = p + 1;
        }
        p++;
    }
    
    /* Handle last cell if no trailing pipe */
    if(col < MAXCOLS && start < p) {
        int len = p - start;
        char *cell = malloc(len + 1);
        if(cell == nil)
            sysfatal("malloc failed");
        
        strncpy(cell, start, len);
        cell[len] = '\0';
        
        /* Trim whitespace */
        while(len > 0 && isspace(cell[len-1]))
            cell[--len] = '\0';
        while(*cell && isspace(*cell)) {
            cell++;
            len--;
        }
        
        row->cells[col].text = cell;
        row->cells[col].width = len;
        row->cells[col].align = -1;
        col++;
    }
    
    row->ncells = col;
    
    /* Process alignment from separator row */
    if(istabledelimiter(line)) {
        for(int i = 0; i < row->ncells; i++) {
            char *cell = row->cells[i].text;
            int len = strlen(cell);
            
            if(len < 2) continue;
            
            /* Check for alignment markers */
            if(cell[0] == ':' && cell[len-1] == ':')
                row->cells[i].align = 0;  /* center */
            else if(cell[len-1] == ':')
                row->cells[i].align = 1;  /* right */
            else
                row->cells[i].align = -1; /* left */
        }
    }
}

void
startcodeblock(Slide *s, char *lang)
{
    CodeBlock *cb;
    
    cb = malloc(sizeof(CodeBlock));
    if(cb == nil)
        sysfatal("malloc failed");
    
    cb->lines = malloc(MAXCODEBLOCKLINES * sizeof(char*));
    if(cb->lines == nil)
        sysfatal("malloc failed");
    
    cb->nlines = 0;
    cb->capacity = MAXCODEBLOCKLINES;
    strncpy(cb->lang, lang, sizeof(cb->lang)-1);
    cb->lang[sizeof(cb->lang)-1] = '\0';
    
    s->current_block = cb;
}

void
addtocodeblock(Slide *s, char *line)
{
    CodeBlock *cb = s->current_block;
    char *newline;
    
    if(cb == nil || cb->nlines >= cb->capacity)
        return;
    
    newline = strdup(line);
    if(newline == nil)
        sysfatal("strdup failed");
    
    cb->lines[cb->nlines++] = newline;
}

void
endcodeblock(Slide *s)
{
    Line *line;
    
    if(s->nlines >= MAXLINES)
        return;
    
    line = &s->lines[s->nlines++];
    memset(line, 0, sizeof(Line));
    line->type = CODEBLOCK;
    line->codeblock = s->current_block;
    s->current_block = nil;
}

void
freecodeblock(CodeBlock *cb)
{
    int i;
    
    if(cb == nil)
        return;
    
    for(i = 0; i < cb->nlines; i++)
        free(cb->lines[i]);
    
    free(cb->lines);
    free(cb);
}

void
processcodeblock(char *line, Slide *current)
{
    char lang[32];
    char *p;
    
    /* Check for code block delimiter */
    if(strncmp(line, "```", 3) == 0) {
        if(current->current_block == nil) {
            /* Start of code block */
            p = line + 3;
            while(*p == '`')
                p++;
            strncpy(lang, p, sizeof(lang)-1);
            lang[sizeof(lang)-1] = '\0';
            startcodeblock(current, lang);
        } else {
            /* End of code block */
            endcodeblock(current);
        }
    } else if(current->current_block != nil) {
        /* Add line to current code block */
        addtocodeblock(current, line);
    }
}

/* Common keywords for syntax highlighting */
char *keywords[] = {
    "if", "else", "for", "while", "do", "break", "continue", "return",
    "switch", "case", "default", "void", "int", "char", "float", "double",
    "struct", "enum", "typedef", "union", "const", "static", "extern",
    "class", "public", "private", "protected", "new", "delete", "this",
    "function", "var", "let", "const", "def", "import", "from", "as",
    nil
};

char *types[] = {
    "int", "char", "float", "double", "void", "bool", "string", "array",
    "String", "Array", "Object", "Number", "Boolean", "Function",
    nil
};

int
iskeyword(char *word)
{
    char **kw;
    for(kw = keywords; *kw != nil; kw++)
        if(strcmp(word, *kw) == 0)
            return 1;
    return 0;
}

int
istype(char *word)
{
    char **t;
    for(t = types; *t != nil; t++)
        if(strcmp(word, *t) == 0)
            return 1;
    return 0;
}

void
drawsyntaxhighlighted(char *line, Point p)
{
    char word[256];
    char *start = line;
    int in_string = 0;
    Rectangle bounds = screen->r;
    
    /* Stop if we're outside window bounds */
    if(p.y < bounds.min.y || p.y + font->height > bounds.max.y)
        return;
    
    while(*line) {
        /* Stop if we exceed window width */
        if(p.x >= bounds.max.x - margin.x)
            break;
            
        /* Handle comments */
        if(!in_string && *line == '/' && *(line+1) == '/') {
            /* Only draw comment if it fits */
            if(p.x + stringwidth(font, line) <= bounds.max.x - margin.x)
                string(screen, p, comment_color, ZP, font, line);
            return;
        }
        
        /* Handle strings */
        if(*line == '"' && (line == start || *(line-1) != '\\')) {
            if(!in_string) {
                start = line;
                in_string = 1;
            } else {
                line++;
                int len = line - start;
                strncpy(word, start, len);
                word[len] = '\0';
                
                /* Only draw string if it fits */
                if(p.x + stringwidth(font, word) <= bounds.max.x - margin.x) {
                    string(screen, p, string_color, ZP, font, word);
                    p.x += stringwidth(font, word);
                }
                in_string = 0;
                start = line;
                continue;
            }
        }
        
        if(!in_string && (*line == ' ' || *line == '\t' || *line == '(' || *line == ')' || 
           *line == '{' || *line == '}' || *line == '[' || *line == ']' || 
           *line == ',' || *line == ';' || *line == '\0')) {
            if(line > start) {
                int len = line - start;
                strncpy(word, start, len);
                word[len] = '\0';
                
                /* Check if word fits */
                int wordwidth = stringwidth(font, word);
                if(p.x + wordwidth <= bounds.max.x - margin.x) {
                    /* Determine word type and color */
                    if(iskeyword(word))
                        string(screen, p, keyword_color, ZP, font, word);
                    else if(istype(word))
                        string(screen, p, type_color, ZP, font, word);
                    else if(*(line) == '(')  /* Function call */
                        string(screen, p, function_color, ZP, font, word);
                    else if(word[0] >= '0' && word[0] <= '9')  /* Number */
                        string(screen, p, number_color, ZP, font, word);
                    else
                        string(screen, p, text, ZP, font, word);
                    
                    p.x += wordwidth;
                }
            }
            
            if(*line && p.x + stringwidth(font, "x") <= bounds.max.x - margin.x) {
                char punct[2] = {*line, '\0'};
                string(screen, p, text, ZP, font, punct);
                p.x += stringwidth(font, punct);
            }
            
            start = line + 1;
        }
        
        if(*line)
            line++;
    }
}

void
drawcodeblock(CodeBlock *cb, Point *p)
{
    Rectangle r;
    int i, y;
    Rectangle bounds = screen->r;
    
    if(cb == nil)
        return;
    
    /* Skip if we're outside window bounds */
    if(p->y < bounds.min.y || p->y >= bounds.max.y - margin.y)
        return;
    
    /* Calculate code block rectangle */
    r = Rect(p->x, p->y, 
             bounds.max.x - margin.x,  /* Constrain to window width */
             p->y + (cb->nlines + 1) * font->height);
             
    /* Clip rectangle to window bounds */
    if(r.max.y > bounds.max.y - margin.y)
        r.max.y = bounds.max.y - margin.y;
    
    /* Draw background if within bounds */
    if(r.min.y < r.max.y && r.max.y <= bounds.max.y - margin.y)
        draw(screen, r, lightgrey, nil, ZP);
    
    /* Draw code lines with syntax highlighting */
    y = p->y + font->height/2;
    for(i = 0; i < cb->nlines; i++) {
        /* Stop if we exceed window bounds */
        if(y + font->height > bounds.max.y - margin.y)
            break;
            
        if(cb->lines[i] != nil) {
            Point linestart = Pt(p->x + 10, y);
            /* Only draw if line start is within window bounds */
            if(linestart.x < bounds.max.x - margin.x)
                drawsyntaxhighlighted(cb->lines[i], linestart);
        }
        y += font->height;
    }
    
    /* Update drawing position */
    p->y = y + font->height/2;
    
    /* Ensure we don't exceed window bounds */
    if(p->y > bounds.max.y - margin.y)
        p->y = bounds.max.y - margin.y;
}

ImageData*
parseimage(char *line)
{
    ImageData *img;
    char *alt_start, *alt_end;
    char *path_start, *path_end;
    
    img = malloc(sizeof(ImageData));
    if(img == nil)
        sysfatal("malloc failed");
    
    memset(img, 0, sizeof(ImageData));
    
    /* Parse alt text between ![...] */
    alt_start = strchr(line, '[');
    if(alt_start == nil) {
        free(img);
        return nil;
    }
    alt_start++; /* Skip the [ */
    
    alt_end = strchr(alt_start, ']');
    if(alt_end == nil) {
        free(img);
        return nil;
    }
    
    /* Copy alt text */
    strncpy(img->alt, alt_start, alt_end - alt_start);
    img->alt[alt_end - alt_start] = '\0';
    
    /* Parse image path between (...) */
    path_start = strchr(alt_end, '(');
    if(path_start == nil) {
        free(img);
        return nil;
    }
    path_start++; /* Skip the ( */
    
    path_end = strchr(path_start, ')');
    if(path_end == nil) {
        free(img);
        return nil;
    }
    
    /* Copy path */
    strncpy(img->path, path_start, path_end - path_start);
    img->path[path_end - path_start] = '\0';
    
    /* Initialize image pointer as nil - will be loaded when displayed */
    img->img = nil;
    
    return img;
}

void
drawimage(ImageData *img, Point *p)
{
    Point pt = *p;
    int width, height;
    Image *image;
    Rectangle r;
    int fd;
    
    if(img == nil)
        return;
    
    /* Load image if not already loaded */
    if(img->img == nil) {
        fd = open(img->path, OREAD);
        if(fd < 0) {
            /* If image load fails, display alt text and error message */
            string(screen, pt, text, ZP, font, img->alt);
            pt.y += font->height;
            string(screen, pt, display->black, ZP, font, "(Image load failed)");
            pt.y += font->height * 2;
            *p = pt;
            return;
        }
        
        image = readimage(display, fd, 0);
        close(fd);
        
        if(image == nil) {
            /* If image load fails, display alt text and error message */
            string(screen, pt, text, ZP, font, img->alt);
            pt.y += font->height;
            string(screen, pt, display->black, ZP, font, "(Image load failed)");
            pt.y += font->height * 2;
            *p = pt;
            return;
        }
        img->img = image;
    }
    
    /* Draw the image */
    width = Dx(img->img->r);
    height = Dy(img->img->r);
    
    /* Scale down if too wide */
    Rectangle bounds = screen->r;
    int max_width = bounds.max.x - bounds.min.x - 2*margin.x - pt.x + bounds.min.x;
    
    /* If image is wider than available space, scale it down proportionally */
    if(width > max_width && max_width > 0) {
        int scale_factor = (max_width * 100) / width;
        width = (width * scale_factor) / 100;
        height = (height * scale_factor) / 100;
    }
    
    /* Create rectangle for drawing */
    r = Rect(pt.x, pt.y, pt.x + width, pt.y + height);
    
    /* Draw the image */
    draw(screen, r, img->img, nil, ZP);
    
    /* Add some space after the image */
    pt.y += height + font->height;
    
    /* Update position */
    *p = pt;
}

void
main(int argc, char *argv[])
{
    char *filename;
    Event ev;
    
    ARGBEGIN {
    default:
        usage();
    } ARGEND
    
    if(argc != 1)
        usage();
    
    filename = argv[0];
    argv0 = argv[0];  /* Set argv0 for usage message */
    
    /* Initialize graphics */
    if(initdraw(nil, nil, "slides") < 0)
        sysfatal("initdraw failed: %r");
    
    /* Default font is set by setupdraw which checks environment variables */
    
    /* Set up display */
    setupdraw();
    
    /* Load slides */
    if(loadslides(filename) < 0)
        sysfatal("cannot load slides: %r");
    
    /* Set up event handling */
    einit(Ekeyboard|Emouse);
    eresized(0);
    
    /* Main event loop */
    for(;;) {
        switch(eread(Ekeyboard|Emouse, &ev)) {
        case Ekeyboard:
            handlekey(ev.kbdc);
            break;
        }
    }
} 