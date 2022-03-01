
#include "compat.h"
#include "kplib.h"

struct icon {
    int32_t width,height;
    intptr_t *pixels;
    unsigned char *mask;
};

int writeicon(FILE *fp, struct icon *ico)
{
    int i;

    Bfprintf(fp,
        "#include \"sdl_inc.h\"\n"
        "#include \"sdlappicon.h\"\n"
        "\n"
    );
    Bfprintf(fp,"static Uint8 sdlappicon_pixels[] = {\n");
    for (i=0;i<ico->width*ico->height;i++) {
        if ((i%6) == 0) Bfprintf(fp,"\t");
        else Bfprintf(fp," ");
        Bfprintf(fp, "0x%08lx,", (long)B_LITTLE32(ico->pixels[i]));
        if ((i%6) == 5) Bfprintf(fp,"\n");
    }
    if ((i%16) > 0) Bfprintf(fp, "\n");
    Bfprintf(fp, "};\n\n");

    Bfprintf(fp,
        "struct sdlappicon sdlappicon = {\n"
        "    %d,%d,    // width,height\n"
        "    sdlappicon_pixels\n"
        "};\n",
        ico->width, ico->height
    );

    return 0;
}

int main(int argc, char **argv)
{
    struct icon icon;

    if (argc<2) {
        Bfprintf(stderr, "generateicon <picture file>\n");
        return 1;
    }

    memset(&icon, 0, sizeof(icon));

    engineCreateAllocator();
    kpzload(argv[1], (intptr_t*)&icon.pixels, &icon.width, &icon.height);
    if (!icon.pixels) {
        Bfprintf(stderr, "Failure loading %s\n", argv[1]);
        return 1;
    }

    writeicon(stdout, &icon);

    xfree(icon.pixels);

    return 0;
}
