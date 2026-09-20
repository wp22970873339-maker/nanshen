/*
 * Apple-style Calculator for 3DS
 * 上屏：显示计算结果
 * 下屏：触屏键盘输入
 * 苹果风格：黑底 + 白字 + 橙色运算符
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <citro2d.h>

#define SCREEN_TOP_W  400
#define SCREEN_BOT_W  320
#define SCREEN_H      240

// 按钮定义
typedef struct {
    float x, y, w, h;
    const char *label;
    int type; // 0=num, 1=func, 2=op
    const char *val;
} Button;

// 颜色（苹果风格）
#define COLOR_BG        C2D_Color32(0, 0, 0, 255)        // 黑底
#define COLOR_NUM       C2D_Color32(51, 51, 51, 255)     // 数字键深灰
#define COLOR_FUNC      C2D_Color32(165, 165, 165, 255)   // 功能键浅灰
#define COLOR_OP        C2D_Color32(255, 149, 0, 255)   // 运算符苹果橙
#define COLOR_TEXT      C2D_Color32(255, 255, 255, 255)  // 白色文字
#define COLOR_TEXT_DARK C2D_Color32(0, 0, 0, 255)         // 黑色文字（浅灰键上）
#define COLOR_EXPR      C2D_Color32(142, 142, 147, 255)  // 算式灰色

// 计算器状态
char current[32] = "0";
char previous[32] = {0};
char op = 0;
bool justCalculated = false;

// 按钮布局（下屏 320x240）
Button buttons[20];

void initButtons(void) {
    // 下屏边距
    float margin = 8.0f;
    float gap = 6.0f;
    float btnW = (SCREEN_BOT_W - margin * 2 - gap * 3) / 4.0f;
    float btnH = (SCREEN_H - margin * 2 - gap * 4) / 5.0f;

    const char *labels[5][4] = {
        {"⌫", "AC", "%", "÷"},
        {"7", "8", "9", "×"},
        {"4", "5", "6", "−"},
        {"1", "2", "3", "+"},
        {"+/−", "0", ".", "="}
    };
    const int types[5][4] = {
        {1, 1, 1, 2},
        {0, 0, 0, 2},
        {0, 0, 0, 2},
        {0, 0, 0, 2},
        {0, 0, 0, 2}
    };
    const char *vals[5][4] = {
        {"del", "C", "%", "/"},
        {"7", "8", "9", "*"},
        {"4", "5", "6", "-"},
        {"1", "2", "3", "+"},
        {"±", "0", ".", "="}
    };

    int idx = 0;
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 4; col++) {
            buttons[idx].x = margin + col * (btnW + gap);
            buttons[idx].y = margin + row * (btnH + gap);
            buttons[idx].w = btnW;
            buttons[idx].h = btnH;
            buttons[idx].label = labels[row][col];
            buttons[idx].type = types[row][col];
            buttons[idx].val = vals[row][col];
            idx++;
        }
    }
}

float calculate(float a, float b, char o) {
    switch (o) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return b == 0 ? 0 : a / b;
    }
    return 0;
}

void updateDisplay(void) {
    // 上屏清屏
    C2D_TargetClear(gfxGetDrawFrame(GFX_TOP), COLOR_BG);

    // 画算式（小字，上方）
    char expr[64];
    if (op) {
        const char *opSym = op == '/' ? "÷" : op == '*' ? "×" : op == '-' ? "−" : "+";
        snprintf(expr, sizeof(expr), "%s %s", previous, opSym);
    } else {
        expr[0] = 0;
    }

    if (expr[0]) {
        C2D_DrawText(expr, C2D_AlignRight,
                     SCREEN_TOP_W - 20, 40, 0.0f,
                     0.6f, 0.6f,
                     COLOR_EXPR);
    }

    // 画结果（大字，下方靠右）
    // 根据数字长度调整字号
    float scale = 1.0f;
    int len = strlen(current);
    if (len > 10) scale = 0.6f;
    else if (len > 8) scale = 0.7f;
    else if (len > 6) scale = 0.8f;
    else if (len > 4) scale = 0.9f;

    C2D_DrawText(current, C2D_AlignRight,
                 SCREEN_TOP_W - 20, 180, 0.0f,
                 scale, scale,
                 COLOR_TEXT);
}

void drawButtons(void) {
    for (int i = 0; i < 20; i++) {
        Button *b = &buttons[i];
        u32 color;
        switch (b->type) {
            case 0: color = COLOR_NUM; break;
            case 1: color = COLOR_FUNC; break;
            default: color = COLOR_OP; break;
        }

        // 画圆角矩形按钮
        C2D_DrawRectSolid(b->x, b->y, 0.0f, b->w, b->h, color);

        // 画文字
        u32 textColor = (b->type == 1) ? COLOR_TEXT_DARK : COLOR_TEXT;
        float cx = b->x + b->w / 2.0f;
        float cy = b->y + b->h / 2.0f;
        C2D_DrawText(b->label, C2D_AlignCenter,
                     cx, cy - 8, 0.0f,
                     0.5f, 0.5f,
                     textColor);
    }
}

int hitTest(float tx, float ty) {
    for (int i = 0; i < 20; i++) {
        Button *b = &buttons[i];
        if (tx >= b->x && tx <= b->x + b->w &&
            ty >= b->y && ty <= b->y + b->h) {
            return i;
        }
    }
    return -1;
}

void onInput(const char *val) {
    if (strlen(val) == 1 && (val[0] >= '0' && val[0] <= '9' || val[0] == '.')) {
        // 数字或小数点
        if (justCalculated) {
            strcpy(current, "0");
            justCalculated = false;
        }
        if (val[0] == '.' && strchr(current, '.')) return;
        if (strcmp(current, "0") == 0 && val[0] != '.') {
            strcpy(current, val);
        } else {
            strncat(current, val, sizeof(current) - strlen(current) - 1);
        }
    } else if (strcmp(val, "+") == 0 || strcmp(val, "-") == 0 ||
               strcmp(val, "*") == 0 || strcmp(val, "/") == 0) {
        // 运算符
        if (previous[0] && op && !justCalculated) {
            float result = calculate(atof(previous), atof(current), op);
            snprintf(current, sizeof(current), "%g", result);
        }
        strcpy(previous, current);
        op = val[0];
        justCalculated = true;
    } else if (strcmp(val, "=") == 0) {
        // 等于
        if (previous[0] && op) {
            float result = calculate(atof(previous), atof(current), op);
            snprintf(current, sizeof(current), "%g", result);
            previous[0] = 0;
            op = 0;
        }
        justCalculated = true;
    } else if (strcmp(val, "C") == 0) {
        // 清除
        strcpy(current, "0");
        previous[0] = 0;
        op = 0;
    } else if (strcmp(val, "del") == 0) {
        // 删除一位
        int len = strlen(current);
        if (len > 1) current[len - 1] = 0;
        else strcpy(current, "0");
    } else if (strcmp(val, "±") == 0) {
        // 正负号
        float v = atof(current);
        snprintf(current, sizeof(current), "%g", -v);
    } else if (strcmp(val, "%") == 0) {
        // 百分号
        float v = atof(current);
        snprintf(current, sizeof(current), "%g", v / 100.0f);
    }
}

int main(void) {
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    // 创建两个屏幕的渲染目标
    C3D_RenderTarget *top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget *bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    initButtons();

    touchPosition touch;

    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();

        if (kDown & KEY_START) break;

        // 检测触屏
        hidTouchRead(&touch);
        if (kDown & KEY_TOUCH) {
            int btn = hitTest((float)touch.px, (float)touch.py);
            if (btn >= 0) {
                onInput(buttons[btn].val);
            }
        }

        // 开始渲染
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

        // 上屏
        C2D_SceneBegin(top);
        updateDisplay();

        // 下屏
        C2D_SceneBegin(bottom);
        C2D_TargetClear(bottom, COLOR_BG);
        drawButtons();

        C3D_FrameEnd(0);
    }

    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}
