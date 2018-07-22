#ifndef H_FRAME
#define H_FRAME

/** frame is a view in local coordinates. */
struct frame {
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    double ratio;
};

/** frame_reset sets fm components to 0. */
void   frame_reset(struct frame* fm);
/** frame_copy copy src into dest. */
void   frame_copy(struct frame* dest, const struct frame* src);
/** frame_set set xmin, xmax, ymin & deduces ymax from them. */
void   frame_set(struct frame* fm, double xmin, double xmax, double ymin);
/** frame_set_ymax set ymax using xmin, xmax & ymin. */
void   frame_set_ymax(struct frame* fm);
/** frame_width returns the width of a frame. */
double frame_width(struct frame* fm);
/** frame_height returns the height of a frame. */
double frame_height(struct frame* fm);
/** frame_translate translates a frame. */
void   frame_translate(struct frame* fm, double x, double y);
/** frame_zoom zooms inside a frame. */
void   frame_zoom(struct frame* fm, double factor);
/** frame_globalx_to_localx transforms a screen x coord to a frame x coord. */
double frame_globalx_to_localx(struct frame* fm, int x, int width);
/** frame_globaly_to_localy transforms a screen y coord to a frame y coord. */
double frame_globaly_to_localy(struct frame* fm, int y, int height);

#endif
