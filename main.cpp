#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <cmath>
#include <unistd.h>
#include <sys/time.h>

Display* display;
Window window;
GC gc;

int ww = 800;
int wh = 600;

void windowInit();
void createGC();
void registerEvents();
void eventLoop();
void handleEvent();

int main() {
    windowInit();
    createGC();
    registerEvents();
    eventLoop();
}

void windowInit() {
    display = XOpenDisplay(NULL);
    if(display == NULL) {
        printf("No Display");
        exit(-1);
    }

    int screenNum = DefaultScreen(display);
    long bg = WhitePixel(display, screenNum);
    long fg = BlackPixel(display, screenNum);
    window = XCreateSimpleWindow(
        display, DefaultRootWindow(display),
        10, 10, ww, wh, 4, fg, bg
    );
    XMapRaised(display, window);
    XSync(display, False);
}

void createGC() {
    gc = XCreateGC(display, window, 0, 0);
    if(gc < 0) {
        printf("No GC...");
        exit(-2);
    }
}

void registerEvents() {
    XSelectInput(
        display, window,
        ButtonPressMask | ButtonReleaseMask | ExposureMask |
        ButtonMotionMask | KeyPressMask
    );
}

Pixmap screen;

void Rect(bool fill, int x, int y, int w, int h, XColor c) {
    XSetForeground(display, gc, c.pixel);
    if(h < 0) {
        h = -h;
        y -= h;
    }
    if(w < 0) {
        w = -w;
        x -= w;
    }
    if(fill) {
        XFillRectangle(display, screen, gc, x, y, w, h);
    } else {
        XDrawRectangle(display, screen, gc, x, y, w, h);
    }
}

void PolyV(bool fill, int x1, int y1, int x2, int y2, int h, XColor c) {
    XSetForeground(display, gc, c.pixel);
    XPoint points[4];
    points[0].x = x1;
    points[0].y = y1;
    points[1].x = x2;
    points[1].y = y2;
    points[2].x = x2;
    points[2].y = y2 + h;
    points[3].x = x1;
    points[3].y = y1 + h;
    if(fill) {
        XFillPolygon(display, screen, gc, points, 4, Nonconvex, CoordModeOrigin);
    } else {
        XDrawLines(display, screen, gc, points, 4, CoordModeOrigin);
    }
}

void PolyH(bool fill, int x1, int y1, int x2, int y2, int w, XColor c) {
    XSetForeground(display, gc, c.pixel);
    XPoint points[4];
    points[0].x = x1;
    points[0].y = y1;
    points[1].x = x2;
    points[1].y = y2;
    points[2].x = x2 + w;
    points[2].y = y2;
    points[3].x = x1 + w;
    points[3].y = y1;
    if(fill) {
        XFillPolygon(display, screen, gc, points, 4, Nonconvex, CoordModeOrigin);
    } else {
        XDrawLines(display, screen, gc, points, 4, CoordModeOrigin);
    }
}

XColor cWhite;
XColor cBlack;
XColor cLightGrey;
XColor cGrey;
XColor cDarkGrey;
XColor cRed;
XColor cGreen;
XColor cBlue;
XColor cLightBrown;
XColor cBrown;

XFontStruct* fMain;
XFontStruct* fTitle;
XFontStruct* fSubTitle;

int P1 = 800;
int P2 = 195;

int score = 0;
int money = 0;
int enemNum = 0;

struct Castle {
    int health, totalHealth;
    int status;
    int x, y;

    Pixmap p;

    Castle() {
        status = 0;
        health = totalHealth = 100;
        x = -20;
        y = wh - 80;
    }

    void drawBody() {
        PolyH(true, x, y - 100, x + 100, y - 180, 40, cGrey);
        PolyH(true, x + 160, y - 100, x + 260, y - 180, 40, cGrey);

        PolyH(true, x, y - 100, x + 20, y - 116, 200, cGrey);
        PolyH(true, x + 100, y - 180, x + 80, y - 164, 200, cGrey);

        PolyH(true, x + 60, y - 116, x + 120, y - 164, 120, cDarkGrey);

        Rect(true, x, y, 200, -100, cGrey);
        PolyV(true, x + 200, y, x + 300, y - 80, -100, cLightGrey);

        PolyH(false, x, y - 100, x + 100, y - 180, 40, cBlack);
        PolyH(false, x + 160, y - 100, x + 260, y - 180, 40, cBlack);

        PolyH(false, x + 60, y - 116, x + 120, y - 164, 120, cBlack);

        PolyH(false, x, y - 100, x + 20, y - 116, 200, cBlack);
        PolyH(false, x + 100, y - 180, x + 80, y - 164, 200, cBlack);

        Rect(false, x, y, 200, -100, cBlack);
        PolyV(false, x + 200, y, x + 300, y - 80, -100, cBlack);
    }

    void drawDoor() {
        PolyV(true, x + 220, y - 16, x + 280, y - 64, -72, cBrown);
        PolyV(false, x + 220, y - 16, x + 280, y - 64, -72, cBlack);
    }

    void drawTurret(int sx, int sy) {
        Rect(true, x + sx, y + sy, 40, -40, cGrey);
        PolyV(true, x + sx + 40, y + sy, x + sx + 60, y + sy - 16, -40, cLightGrey);
        PolyH(true, x + sx, y + sy - 40, x + sx + 20, y + sy - 56, 40, cGrey);
        Rect(false, x + sx, y + sy, 40, -40, cBlack);
        PolyV(false, x + sx + 40, y + sy, x + sx + 60, y + sy - 16, -40, cBlack);
        PolyH(false, x + sx, y + sy - 40, x + sx + 20, y + sy - 56, 40, cBlack);
    }

    void drawHealth() {
        char hp[256];
        sprintf(hp, "Health: %d/%d", health, totalHealth);

        XSetForeground(display, gc, cBlack.pixel);
        XSetFont(display, gc, fMain->fid);
        XDrawString(display, screen, gc, 14, 18, hp, strlen(hp));

        Rect(true, 10, 25, (health * 300) / totalHealth, 25, cRed);
        Rect(false, 10, 25, 300, 25, cBlack);
    }

    void draw() {
        drawBody();
        drawDoor();
        drawTurret(80, -164);
        drawTurret(240, -164);
        drawTurret(0, -100);
        drawTurret(160, -100);

        drawHealth();
    }

    void damage(int amount) {
        health -= amount;
        if(health <= 0) {
            health = 0;
            status = 1;
        }
    }

    void reset() {
        health = 100;
        totalHealth = 100;
    }

    bool isDead() {
        return health <= 0;
    }
};

Castle castle;

struct Enemy {
    int pos, vel, dam, ats, af, plane, px, py,
        state, height, fall, death, r, type;

    Enemy* next;
    Enemy* prev;

    Enemy(int t) {
        type = t;
        pos = P1;
        r = (rand() % 4);
        plane = wh - (r * 20 + 90);
        if(type == 0) {
            vel = 2;
            dam = 1;
            ats = 20;
        } else if(type == 1) {
            vel = 1;
            dam = 10;
            ats = 100;
        }
        fall = 20;
        px = py = height = -1;
        state = 0;
        next = prev = NULL;
        death = 0;
        af;
    }

    void draw() {
        int x, y;
        if(state == 0) {
            x = pos;
            y = plane;
        } else {
            x = px;
            y = py;
        } 

        if(state == 3) {
            XSetForeground(display, gc, cRed.pixel);

            XFillArc(display, screen, gc, x, y - 5, 15, 5, 0, 360*64);
        } else {
            if(type == 1) {
                Rect(true, x - 11, y - 16, 30, 3, cLightBrown);
                Rect(false, x - 11, y - 16, 30, 3, cBlack);
            }

            XSetForeground(display, gc, cWhite.pixel);
            XFillArc (display, screen, gc, x + 1, y - 28, 8, 8, 0, 360*64);
            
            XSetForeground(display, gc, cBlack.pixel);

            XDrawLine(display, screen, gc, x + 5, y - 12, x, y);
            XDrawLine(display, screen, gc, x + 5, y - 12, x + 10, y);
            XDrawLine(display, screen, gc, x + 5, y - 12, x + 5, y - 20);
            XDrawLine(display, screen, gc, x + 5, y - 20, x + 1, y - 14);
            XDrawLine(display, screen, gc, x + 5, y - 20, x + 8, y - 14);
            XDrawArc (display, screen, gc, x + 1, y - 28, 8, 8, 0, 360*64);
            XFillRectangle(display, screen, gc, x + 3, y - 25, 2, 2);
        }
    }

    int move() {
        if(state == 0) {
            if(pos <= P2 + 25*r) {
                af++;
                if(af > ats) {
                    castle.damage(dam);
                    af = 0;
                }
            } else {
                pos -= vel;
                if(pos <= P2 + 25*r) {
                    pos = P2 + 25*r;
                }
            }
        } else if(state == 1) {
            // Being Dragged, do nothing
        } else if(state == 2) {
            // Falling
            if(py >= plane) {
                state = 0;
                if(plane - height > 175) {
                    if(type == 0) {
                        score += 100;
                        money += 100;
                    } else if(type == 1) {
                        score += 250;
                        money += 250;
                    }
                    state = 3;
                } else {
                    pos = px;
                    if(pos < P2 + 25*r) {
                        pos = P2 + 25*r;
                    }
                }
            } else {
                py += fall;
            }
        } else if(state == 3) {
            death++;
            if(death > 1000) {
                return 2;
            }
            return 1;
        }
        return 0;
    }

    bool hit(int x, int y) {
        if(state == 3) {
            return false;
        }
        int hx, hy;
        if(state == 0) {
            hx = pos;
            hy = plane;
        } else {
            hx = px;
            hy = py;
        }
        if(x > hx - 10 && x < hx + 20 && y < hy && y > hy - 28) {
            onmove(x, y);
            return true;
        }
        return false;
    }

    void onmove(int x, int y) {
        state = 1;
        px = x;
        py = y;
    }

    void onrelease(int x, int y) {
        state = 2;
        height = y;
        px = x;
        py = y;
    }
};

struct EnemyList {
    Enemy* first;
    Enemy* last;
};

int currentState = 0;
EnemyList el;
EnemyList dl;

void eventLoop() {
    int depth = DefaultDepth(display, DefaultScreen(display));
    
    Visual* v = DefaultVisual(display, DefaultScreen(display));
    Colormap cm = XCreateColormap(
        display, window, v, AllocNone
    );

    cBlack.red = cBlack.green = cBlack.blue = 0;
    XAllocColor(display, cm, &cBlack);

    cWhite.red = cWhite.green = cWhite.blue = 65535;
    XAllocColor(display, cm, &cWhite);

    cLightGrey.red = cLightGrey.green = cLightGrey.blue = 48000;
    XAllocColor(display, cm, &cLightGrey);
    cGrey.red = cGrey.green = cGrey.blue = 33000;
    XAllocColor(display, cm, &cGrey);
    cDarkGrey.red = cDarkGrey.green = cDarkGrey.blue = 8000;
    XAllocColor(display, cm, &cDarkGrey);

    cRed.red = cGreen.green = cBlue.blue = 65535;
    cRed.green = cRed.blue = cGreen.red = cGreen.blue = cBlue.red = cBlue.green = 0;
    XAllocColor(display, cm, &cRed);
    XAllocColor(display, cm, &cGreen);
    XAllocColor(display, cm, &cBlue);

    cBrown.red = 30000;
    cBrown.green = 20000;
    cBrown.blue = 5000;
    XAllocColor(display, cm, &cBrown);

    cLightBrown.red = 60000;
    cLightBrown.green = 40000;
    cLightBrown.blue = 10000;
    XAllocColor(display, cm, &cLightBrown);

    fTitle = XLoadQueryFont(display, "*-helvetica*bold-r*34*");
    fMain = XLoadQueryFont(display, "*-helvetica-*-r-*18-*");

    
    screen = XCreatePixmap(display, window, ww, wh, depth);

    Pixmap bgImg;
    unsigned int temp1, temp2;
    int temp3, temp4;
    XReadBitmapFile(
        display, window, "bg.xbm",
        &temp1, &temp2, &bgImg, &temp3, &temp4
    );

    Enemy* en;
    el.first = NULL;
    el.last = NULL;
    dl.first = dl.last = NULL;
    int rate = -1;
    int typeChance = 0;

    while(1) {
        while(XPending(display)) {
            handleEvent();
        }
        
        if(currentState == 0) {
            XSetForeground(display, gc, cGreen.pixel);
            XSetBackground(display, gc, cBlue.pixel);
            XCopyPlane(
                display, bgImg, screen, gc,
                0, 0, ww, wh, 0, 0, 1
            );

            castle.draw();

            char t[] = "Defend Your Castle";
            XSetForeground(display, gc, cBlack.pixel);
            XSetFont(display, gc, fTitle->fid);
            XDrawString(display, screen, gc, 300, 200, t, strlen(t));

            char st[] = "Lovingly ripped off of the popular flash game";
            XSetFont(display, gc, fMain->fid);
            XDrawString(display, screen, gc, 350, 220, st, strlen(st));

            char rules1[] = "Stick figures are attacking your castle!";
            char rules2[] = "To kill them, drag them high up and drop them.";
            char rules3[] = "If they make it to your gates, they will pound them down!";
            char rules4[] = "Normal units give you $100. Strong ones give you $250!";
            char rules5[] = "The hoards never end. Instead they grow ever stronger!";
            char rules6[] = "Play until you die.";
            char rules7[] = "Pressing 'h' heals you by 10 health and costs $100.";
            char rules8[] = "Pressing 'f' fortifies your castle by 100 health and costs $5000.";
            char rules9[] = "Click anywhere to start.";
            char rules10[] = "Hit Escape at any time to quit.";
            char rules11[] = "Oh, and please go easy on your wrist; it's useful!!";
            XDrawString(display, screen, gc, 300, 350, rules1, strlen(rules1));
            XDrawString(display, screen, gc, 300, 370, rules2, strlen(rules2));
            XDrawString(display, screen, gc, 300, 390, rules3, strlen(rules3));
            XDrawString(display, screen, gc, 300, 410, rules4, strlen(rules4));
            XDrawString(display, screen, gc, 300, 430, rules5, strlen(rules5));
            XDrawString(display, screen, gc, 300, 450, rules6, strlen(rules6));
            XDrawString(display, screen, gc, 275, 470, rules7, strlen(rules7));
            XDrawString(display, screen, gc, 250, 490, rules8, strlen(rules8));
            XDrawString(display, screen, gc, 225, 510, rules9, strlen(rules9));
            XDrawString(display, screen, gc, 200, 530, rules10, strlen(rules10));
            XDrawString(display, screen, gc, 175, 550, rules11, strlen(rules11));

            
        } else if(currentState == 1) {
            rate = ((score / 1000) + 1) * 150;
            typeChance = (score / 1000) * 5;
            if(typeChance > 50) {
                typeChance = 50;
            }

            if(rate > 1000 && (score / 1000) % 6 != 1) {
                rate = 650;
            }

            int r = rand() % 10000;
            if(r < rate) {
                Enemy* n = new Enemy((rand() % 100) < typeChance);
                enemNum++;
                if(el.last) {
                    el.last->next = n;
                    n->prev = el.last;
                }
                el.last = n;
                if(!el.first) {
                    el.first = n;
                }
            }
            
            en = dl.first;
            while(en) {
                bool state = en->move();
                if(state == 2) {
                    Enemy* d = en;
                    if(d->prev) {
                        d->prev->next = d->next;
                    }
                    if(d->next) {
                        d->next->prev = d->prev;
                    }
                    if(dl.first == d) {
                        dl.first = d->next;
                    }
                    if(dl.last == d) {
                        dl.last = d->prev;
                    }
                    en = d->next;
                    delete d;
                } else {
                    en = en->next;
                }
            }

            en = el.first;
            while(en) {
                bool state = en->move();
                if(state == 1) {
                    Enemy* d = en;
                    if(d->prev) {
                        d->prev->next = d->next;
                    }
                    if(d->next) {
                        d->next->prev = d->prev;
                    }
                    if(el.first == d) {
                        el.first = d->next;
                    }
                    if(el.last == d) {
                        el.last = d->prev;
                    }
                    en = d->next;
                    d->next = NULL;
                    if(dl.last) {
                        dl.last->next = d;
                        d->prev = dl.last;
                    }
                    dl.last = d;
                    if(!dl.first) {
                        dl.first = d;
                    }
                } else {
                    en = en->next;
                }
            }

            if(castle.isDead()) {
                currentState = 2;
            }

            XSetForeground(display, gc, cGreen.pixel);
            XSetBackground(display, gc, cBlue.pixel);
            XCopyPlane(
                display, bgImg, screen, gc,
                0, 0, ww, wh, 0, 0, 1
            );

            en = dl.first;
            while(en) {
                en->draw();
                en = en->next;
            }

            castle.draw();

            en = el.first;
            while(en) {
                en->draw();
                en = en->next;
            }
        } else if(currentState == 2) {
            char go[] = "Game Over";
            XSetForeground(display, gc, cBlack.pixel);
            XSetFont(display, gc, fTitle->fid);
            XDrawString(display, screen, gc, 300, 200, go, strlen(go));

            char re[] = "Hit Enter to restart";
            XSetFont(display, gc, fMain->fid);
            XDrawString(display, screen, gc, 320, 220, re, strlen(re));
        }

        char sc[256];
        sprintf(sc, "Score: %d", score);
        XSetForeground(display, gc, cBlack.pixel);
        XSetFont(display, gc, fMain->fid);
        XDrawString(display, screen, gc, 13, 75, sc, strlen(sc));

        sprintf(sc, "$%d", money);
        XSetForeground(display, gc, cBlack.pixel);
        XSetFont(display, gc, fMain->fid);
        XDrawString(display, screen, gc, 13, 90, sc, strlen(sc));

        XCopyArea(
            display, screen, window, gc,
            0, 0, ww, wh, 0, 0
        );
        XFlush(display);

        usleep(60000);
    }
}

Enemy* drag = NULL;

void handleEvent() {
    XEvent e;
    
    int key;
    XNextEvent(display, &e);
    switch(e.type) {
        case KeyPress:
            // Escape Key Exits
            key = XKeycodeToKeysym(display, e.xkey.keycode, 0);
            if(key == 0xff1b) {
                exit(0);
            } else if(key == 0xff0d && currentState == 2) {
                currentState = 1;
                Enemy* en = el.first;
                while(en) {
                    Enemy* d = en;
                    en = en->next;
                    delete d;
                }

                en = dl.first;
                while(en) {
                    Enemy* d = en;
                    en = en->next;
                    delete d;
                }

                el.first = el.last = new Enemy(0);
                dl.first = dl.last = NULL;
                castle.reset();
                score = enemNum = 0;
            }

            if(currentState == 1) {
                if(key == 0x68) {
                    // 'h' key
                    if(money >= 100 && castle.health < castle.totalHealth) {
                        money -= 100;
                        castle.health += 10;
                        if(castle.health > castle.totalHealth) {
                            castle.health = castle.totalHealth;
                        }
                    }
                } else if(key == 0x66) {
                    if(money >= 5000) {
                        money -= 5000;
                        castle.totalHealth += 100;
                    }
                }
            }
            break;

        case ButtonPress:
            if(currentState == 0) {
                currentState = 1;
                Enemy* en = el.first;
                while(en) {
                    Enemy* d = en;
                    en = en->next;
                    delete d;
                }
                
                en = dl.first;
                while(en) {
                    Enemy* d = en;
                    en = en->next;
                    delete d;
                }

                el.first = el.last = new Enemy(0);
                dl.first = dl.last = NULL;
                castle.reset();
                score = enemNum = 0;
            } else if(currentState == 1) {
                Enemy* en = el.first;
                while(en) {
                    if(en->hit(e.xbutton.x, e.xbutton.y)) {
                        drag = en;
                        break;
                    }
                    en = en->next;
                }
            }
            break;

        case MotionNotify:
            if(drag) {
                drag->onmove(e.xmotion.x, e.xmotion.y);
            }
            break;

        case ButtonRelease:
            if(drag) {
                drag->onrelease(e.xbutton.x, e.xbutton.y);
                drag = NULL;
            }
            break;

        case Expose:
            break;
    }
}

