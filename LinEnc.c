
#include<stdio.h>
typedef unsigned char      U8;
typedef unsigned short     U16;
typedef unsigned int       U32;
typedef unsigned long long U64;

typedef signed char        S8;
typedef signed short       S16;
typedef signed int         S32;
typedef signed long long   S64;

typedef float              F32;
typedef double             F64;

//add look-up table for linear interpolation delay to pitch angle
#define TABLE_SIZE 21
struct InterpolationData
{
    S32 position_us;
    S32 positionDeg;
};
static struct InterpolationData linearEncLookUpTable[TABLE_SIZE];

static void initLookUpTable(void)
{
   linearEncLookUpTable[0].positionDeg = -1000;
   linearEncLookUpTable[0].position_us = 2989;
   linearEncLookUpTable[1].positionDeg = 0;
   linearEncLookUpTable[1].position_us = 3221;
   linearEncLookUpTable[2].positionDeg = 200;
   linearEncLookUpTable[2].position_us = 3662;
   linearEncLookUpTable[3].positionDeg = 400;
   linearEncLookUpTable[3].position_us = 4122;
   linearEncLookUpTable[4].positionDeg = 600;
   linearEncLookUpTable[4].position_us = 4564;
   linearEncLookUpTable[5].positionDeg = 800;
   linearEncLookUpTable[5].position_us = 5006;
   linearEncLookUpTable[6].positionDeg = 1000;
   linearEncLookUpTable[6].position_us = 5466;
   linearEncLookUpTable[7].positionDeg = 1200;
   linearEncLookUpTable[7].position_us = 5908;
   linearEncLookUpTable[8].positionDeg = 1400;
   linearEncLookUpTable[8].position_us = 6368;
   linearEncLookUpTable[9].positionDeg = 1600;
   linearEncLookUpTable[9].position_us = 6809;
   linearEncLookUpTable[10].positionDeg = 1800;
   linearEncLookUpTable[10].position_us = 7251;
   linearEncLookUpTable[11].positionDeg = 2000;
   linearEncLookUpTable[11].position_us = 7691;
   linearEncLookUpTable[12].positionDeg = 2500;
   linearEncLookUpTable[12].position_us = 8797;
   linearEncLookUpTable[13].positionDeg = 2720;
   linearEncLookUpTable[13].position_us = 9239;
   linearEncLookUpTable[14].positionDeg = 3000;
   linearEncLookUpTable[14].position_us = 9893;
   linearEncLookUpTable[15].positionDeg = 3500;
   linearEncLookUpTable[15].position_us = 10910;
   linearEncLookUpTable[16].positionDeg = 4000;
   linearEncLookUpTable[16].position_us = 11960;
   linearEncLookUpTable[17].positionDeg = 5000;
   linearEncLookUpTable[17].position_us = 13900;
   linearEncLookUpTable[18].positionDeg = 6000;
   linearEncLookUpTable[18].position_us = 15630;
   linearEncLookUpTable[19].positionDeg = 8200;
   linearEncLookUpTable[19].position_us = 18720;
   linearEncLookUpTable[20].positionDeg = 9500;
   linearEncLookUpTable[20].position_us = 20272;
}

static void interpolateEncoderData( S32 actualPos_us, S32* angleActual,
                                                    struct InterpolationData *enc )
{
    int index = 1;

    if( actualPos_us < enc[0].position_us
        ||
        actualPos_us > enc[TABLE_SIZE - 1].position_us )
    {
        printf( "Linear Enc Out Off Range" );
        printf( "%i", actualPos_us );
    }
    else
    {
        for( ; enc[index].position_us < actualPos_us; index++ );

        *angleActual =
           (S32)( (S64)(enc[index].positionDeg - enc[index - 1].positionDeg ) *
                       (S64)( actualPos_us - enc[index - 1].position_us ) /
                     ( enc[index].position_us - enc[index - 1].position_us ) +
                                                  enc[index - 1].positionDeg );
    }

//    printf( "actualPos %i; Index %i", actualPos_us, index );
//    printf( "maxPos %i; minPos %i", enc[TABLE_SIZE - 1].position_us,
//                                                          enc[0].position_us );
}
int main(void)
{
   FILE *fileDesc;
   fileDesc = fopen("linEnc.csv", "w");

   fprintf(fileDesc, "period,angle\n");
   initLookUpTable();
   S32 actualPos_us = linearEncLookUpTable[0].position_us;
   static S32 angleCalc;
   static S32 *angleActual = &angleCalc;

   while( actualPos_us < linearEncLookUpTable[TABLE_SIZE - 1].position_us )
   {
      interpolateEncoderData( actualPos_us, angleActual, linearEncLookUpTable );

      fprintf( fileDesc, "%i,%i\n", actualPos_us, angleCalc);
      actualPos_us += 1;
   }

   fclose(fileDesc);
   return 0;
}
