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
    back = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0xFAFAFAFF);  /* Off-white background */
    text = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x2C3E50FF);  /* Dark blue-grey text */
    
    /* Headers and accents */
    accent = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x3498DBFF); /* Bright blue */
    dim = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x7F8C8DFF);   /* Muted grey */
    
    /* Special elements */
    quote = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0xECF0F1FF);  /* Light grey for quotes */
    table = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0xBDC3C7FF);  /* Medium grey for tables */
    lightgrey = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0xF5F6FAFF); /* Very light grey for code */
    
    /* Header colors */
    h1color = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0xE74C3CFF); /* Red */
    h2color = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x2980B9FF); /* Dark blue */
    h3color = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x27AE60FF); /* Green */
    h4color = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x8E44ADFF); /* Purple */
    h5color = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0xD35400FF); /* Orange */
    h6color = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x16A085FF); /* Teal */
    
    /* Syntax highlighting colors */
    keyword_color = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0xC0392BFF);  /* Dark red */
    string_color = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x27AE60FF);   /* Green */
    number_color = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0xE67E22FF);   /* Orange */
    comment_color = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x7F8C8DFF);  /* Grey */
    type_color = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x2980B9FF);     /* Blue */
    function_color = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x8E44ADFF);  /* Purple */
    
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
    if(display == nil)
        sysfatal("display not initialized");
    
    screen = display->image;
    if(screen == nil)
        sysfatal("screen not initialized");
    
    /* Use system default font as base font */
    font = display->defaultfont;
    if(font == nil)
        sysfatal("cannot load default font");
    
    /* Try to load larger fonts for headers - try both pelm and lucsans */
    Font *font24 = openfont(display, "/lib/font/bit/pelm/unicode.24.font");
    if(font24 == nil)
        font24 = openfont(display, "/lib/font/bit/lucsans/unicode.24.font");
    
    Font *font20 = openfont(display, "/lib/font/bit/pelm/unicode.20.font");
    if(font20 == nil)
        font20 = openfont(display, "/lib/font/bit/lucsans/unicode.20.font");
    
    Font *font16 = openfont(display, "/lib/font/bit/pelm/unicode.16.font");
    if(font16 == nil)
        font16 = openfont(display, "/lib/font/bit/lucsans/unicode.16.font");
    
    Font *font14 = openfont(display, "/lib/font/bit/pelm/unicode.14.font");
    if(font14 == nil)
        font14 = openfont(display, "/lib/font/bit/lucsans/unicode.14.font");
    
    /* Fallback to default font if larger sizes not available, but scale spacing accordingly */
    h1font = font24 != nil ? font24 : font;
    h2font = font20 != nil ? font20 : font;
    h3font = font16 != nil ? font16 : font;
    h4font = font14 != nil ? font14 : font;
    
    /* Load bold and italic variants of the default font size */
    h5font = bold = openfont(display, "/lib/font/bit/lucsans/typebold.14.font");
    if(h5font == nil)
        h5font = bold = font;
    
    h6font = italic = openfont(display, "/lib/font/bit/lucsans/typeitalic.14.font");
    if(h6font == nil)
        h6font = italic = font;
    
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
freeline(Line *line)
{
    TextSegment *seg = line->segments;
    while(seg != nil) {
        TextSegment *next = seg->next;
        free(seg);
        seg = next;
    }
    line->segments = nil;
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
    USED(maxwidth);  /* Mark parameter as intentionally unused */
    string(screen, p, col, ZP, f, text);
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
    int content_width;
    
    if(n < 0 || n >= nslides)
        return;
    
    /* Calculate content bounds based on window size */
    bounds = screen->r;
    content_width = bounds.max.x - bounds.min.x - 2*margin.x;
    
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
                p.y += 20;  /* More space before H1 */
                string(screen, p, h1color, ZP, h1font, line->text);
                p.y += 2.5 * font->height;  /* More space after H1 */
            } else {
                p.y = bounds.max.y;  /* Force exit from loop */
            }
            break;
            
        case HEADER2:
            if(p.y + h2font->height <= bounds.max.y - margin.y) {
                p.y += 15;  /* Space before H2 */
                string(screen, p, h2color, ZP, h2font, line->text);
                p.y += 2 * font->height;
            } else {
                p.y = bounds.max.y;
            }
            break;
            
        case HEADER3:
            if(p.y + h3font->height <= bounds.max.y - margin.y) {
                p.y += 10;  /* Space before H3 */
                string(screen, p, h3color, ZP, h3font, line->text);
                p.y += 1.5 * font->height;
            } else {
                p.y = bounds.max.y;
            }
            break;
            
        case HEADER4:
            if(p.y + h4font->height <= bounds.max.y - margin.y) {
                p.y += 5;  /* Small space before H4 */
                string(screen, p, h4color, ZP, h4font, line->text);
                p.y += 1.2 * font->height;
            } else {
                p.y = bounds.max.y;
            }
            break;
            
        case HEADER5:
            if(p.y + h5font->height <= bounds.max.y - margin.y) {
                string(screen, p, h5color, ZP, h5font, line->text);
                p.y += 1.1 * font->height;
            } else {
                p.y = bounds.max.y;
            }
            break;
            
        case HEADER6:
            if(p.y + h6font->height <= bounds.max.y - margin.y) {
                string(screen, p, h6color, ZP, h6font, line->text);
                p.y += font->height;
            } else {
                p.y = bounds.max.y;
            }
            break;
            
        case LISTITEM:
            if(p.y + font->height <= bounds.max.y - margin.y) {
                string(screen, p, display->black, ZP, font, "•");
                p.x += 20;
                if(p.x < bounds.max.x - margin.x)
                    string(screen, p, display->black, ZP, font, line->text);
                p.y += font->height;
            } else {
                p.y = bounds.max.y;
            }
            break;
            
        case BLOCKQUOTE:
            if(p.y + font->height <= bounds.max.y - margin.y) {
                Rectangle quote_bar = Rect(p.x - 5, p.y, p.x - 2, p.y + font->height);
                if(quote_bar.max.x <= bounds.max.x - margin.x) {
                    draw(screen, quote_bar, display->black, nil, ZP);
                    string(screen, p, display->black, ZP, italic, line->text);
                }
                p.y += font->height;
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
            
        default:
            if(p.y + font->height <= bounds.max.y - margin.y) {
                string(screen, p, display->black, ZP, font, line->text);
                p.y += font->height;
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
    
    /* Load font */
    font = openfont(display, "/lib/font/bit/pelm/unicode.9.font");
    if(font == nil)
        sysfatal("cannot load font: %r");
    
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