/**
 * Based on RIOT test for display driver pcd8544.
 */
#ifndef RIOTOIR_RIOTOIR_DISPLAY_H
#define RIOTOIR_RIOTOIR_DISPLAY_H

int contrast( int argc, char** argv );

int temp( int argc, char** argv );

int bias( int argc, char** argv );

int on( int argc, char** argv );

int off( int argc, char** argv );

int clear( int argc, char** argv );

int invert( int argc, char** argv );

int riot( int argc, char** argv );

int write( int argc, char** argv );

int display_init(void);


#endif //RIOTOIR_RIOTOIR_DISPLAY_H
