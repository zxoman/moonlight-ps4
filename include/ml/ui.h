#ifndef ML_UI_H
#define ML_UI_H

int ml_ui_init(int width, int height);
void ml_ui_shutdown(void);
void ml_ui_clear(unsigned int rgb);
void ml_ui_text(int x, int y, unsigned int rgb, const char *text);
void ml_ui_present(void);
int ml_ui_width(void);
int ml_ui_height(void);

#endif
