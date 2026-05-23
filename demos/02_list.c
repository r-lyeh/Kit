// enumerate all directories in title storage

#include "kit.h"

void main(event ev) {
   array_(char*) iter = archive.dir(os.argc() > 1 ? os.argv(1) : "*", NULL); // use "**" for subdirs
   for( int i = 0; i < array_count(iter); ++i ) puts(iter[i]);
   printf("%d items\n", array_count(iter));

   app.quit(0);
}

const char *hints;
