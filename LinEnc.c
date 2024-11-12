
#include<stdio.h>
#include<math.h>


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
    S32 positionCount;
    S32 positionMM;
};
static struct InterpolationData linearEncLookUpTable[TABLE_SIZE];

static void initLookUpTable(void)
{
   linearEncLookUpTable[0].positionDeg = -200;
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

#define US 0
#define COUNTS 1
#define MM 1
static void interpolateEncoderData( S32 actualPos,
                                    S32* angleActual,
                                    struct InterpolationData *enc,
                                    U8 us_counts_selector )
{
    int index = 1;

    if( US == us_counts_selector )
    {
       if( actualPos < enc[0].position_us
           ||
           actualPos > enc[TABLE_SIZE - 1].position_us )
       {
           printf( "Linear Enc Out Off Range\r\n" );
           printf( "Line %i Value: %i\r\n", __LINE__, actualPos );
       }
       else
       {
           for( ; enc[index].position_us < actualPos; index++ );

           *angleActual =
           (S32)( (S64)(enc[index].positionDeg - enc[index - 1].positionDeg ) *
                          (S64)( actualPos - enc[index - 1].position_us ) /
                     ( enc[index].position_us - enc[index - 1].position_us ) +
                                                  enc[index - 1].positionDeg );
       }
    }
    else if( COUNTS == us_counts_selector )
    {
       if( actualPos < enc[0].positionCount
           ||
           actualPos > enc[TABLE_SIZE - 1].positionCount )
       {
           printf( "Linear Enc Out Off Range\r\n" );
           printf( "Line %i Value: %i\r\n", __LINE__, actualPos );
       }
       else
       {
           for( ; enc[index].positionCount < actualPos; index++ );

           *angleActual =
           (S32)( (S64)(enc[index].positionDeg - enc[index - 1].positionDeg ) *
                    (S64)( actualPos - enc[index - 1].positionCount ) /
                  ( enc[index].positionCount - enc[index - 1].positionCount ) +
                                               enc[index - 1].positionDeg );
       }
    }
    else if( MM == us_counts_selector )
    {
       if( actualPos < enc[0].positionMM
           ||
           actualPos > enc[TABLE_SIZE - 1].positionMM )
       {
           printf( "Linear Enc Out Off Range\r\n" );
           printf( "Line %i Value: %i\r\n", __LINE__, actualPos );
       }
       else
       {
           for( ; enc[index].positionMM < actualPos; index++ );

           *angleActual =
           (S32)( (S64)(enc[index].positionDeg - enc[index - 1].positionDeg ) *
                    (S64)( actualPos - enc[index - 1].positionMM ) /
                  ( enc[index].positionMM - enc[index - 1].positionMM ) +
                                               enc[index - 1].positionDeg );
       }
    }

//    printf( "actualPos %i; Index %i", actualPos, index );
//    printf( "maxPos %i; minPos %i", enc[TABLE_SIZE - 1].position_us,
//                                                          enc[0].position_us );
}

static void countToMM(S32 actualCounts, U16 entrie)
{
   static const U32 P_HubLinearEncInMinBlade = 1435;
   static const U32 P_HubLinearEncOutMaxBlade = 47500;
   static const U32 P_HubLinearEncOutMinBlade = 0;
   static const U32 P_HubLinearEncInMaxBlade = 9731;
   static const U32 P_HubLinearEncOffsetBlade = 0;

   linearEncLookUpTable[entrie].positionMM =
         ( S32 )( ( U64 ) ( ( U64 ) actualCounts - P_HubLinearEncInMinBlade )
      * ( ( U64 ) P_HubLinearEncOutMaxBlade - P_HubLinearEncOutMinBlade )
      / ( U64 ) ( P_HubLinearEncInMaxBlade - P_HubLinearEncInMinBlade )
      + P_HubLinearEncOutMinBlade ) + P_HubLinearEncOffsetBlade;
}
//resolution at 48MHz 0.0572mm
int main(void)
{
   FILE *fileDesc;
   fileDesc = fopen("linEnc.csv", "w");

   fprintf(fileDesc, "period,counts,angle_us,angleCounts,angleMM,MM\n");
   initLookUpTable();
   S32 actualPos_us = linearEncLookUpTable[0].position_us;
   S32 actualPosMM = linearEncLookUpTable[0].positionMM;

   static S32 angleCalcUs;
   static S32 angleCalcCounts;
   static S32 angleCalcMM;

   U32 cnt = 0;
   static const U8 countsPerUs = 48;
   while( TABLE_SIZE > cnt )
   {
      linearEncLookUpTable[cnt].positionCount =
      (S32)(round((F32)linearEncLookUpTable[cnt].position_us
                     / 100.0 * countsPerUs));

      countToMM( linearEncLookUpTable[cnt].positionCount, cnt );
      printf("%i\t",linearEncLookUpTable[cnt].positionCount);
      printf("%i\n",linearEncLookUpTable[cnt].positionMM);
      cnt++;
   }
   static S32 actualPosCounts;
   actualPosCounts = linearEncLookUpTable[0].positionCount;
//   printf("%i\n",linearEncLookUpTable[TABLE_SIZE - 1].positionCount);

   while( linearEncLookUpTable[TABLE_SIZE - 1].positionCount >= actualPosCounts )
   {
      interpolateEncoderData( actualPos_us, &angleCalcUs,
                              linearEncLookUpTable, US );
      interpolateEncoderData( actualPosCounts, &angleCalcCounts,
                              linearEncLookUpTable, COUNTS );
      interpolateEncoderData( actualPosCounts, &angleCalcMM,
                              linearEncLookUpTable, MM );

      fprintf( fileDesc, "%i,%i,%i,%i,%i,%i\n", actualPos_us, actualPosCounts,
               angleCalcUs, angleCalcCounts, angleCalcMM, actualPosMM );

      actualPosCounts += 1;
      //actualPos_us scale 100
      actualPos_us = (S32)round( (F32)100.0 * actualPosCounts
                                                      / (F32)countsPerUs );
      //actualPosMM scale 100
      actualPosMM = (S32)round(
                  (F32)(actualPosCounts - linearEncLookUpTable[0].positionCount)
                                 * 5.725650916104146576663452266152 );
   }
/**/
   fclose(fileDesc);
   return 0;
}
