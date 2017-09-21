
/*
** Pragmas
*/

/*
** Include Files
*/
#include "cfe_tbl_filedef.h"
#include "pmc_tbldefs.h"

/*
** Local Defines
*/

/*
** Local Structure Declarations
*/
static CFE_TBL_FileDef_t CFE_TBL_FileDef =
{
    /* Content format: ObjName[64], TblName[38], Desc[32], TgtFileName[20], ObjSize
    **    ObjName - variable name of config table, e.g., CI_ConfigDefTbl[]
    **    TblName - app's table name, e.g., CI.CONFIG_TBL, where CI is the same app name
    **              used in cfe_es_startup.scr, and CI_defConfigTbl is the same table
    **              name passed in to CFE_TBL_Register()
    **    Desc - description of table in string format
    **    TgtFileName[20] - table file name, compiled as .tbl file extension
    **    ObjSize - size of the entire table
    */

    "PMC_ConfigTbl", "PMC.CONFIG_TBL", "PMC default config table",
    "pmc_config.tbl", (sizeof(PMC_ConfigTblEntry_t) * PMC_CONFIG_TABLE_MAX_ENTRIES)
};

/*
** External Global Variables
*/

/*
** Global Variables
*/

/* Default CI config table data */
PMC_ConfigTblEntry_t PMC_ConfigTbl[PMC_CONFIG_TABLE_MAX_ENTRIES] =
{
    900,  /* PwmDisarmed */
    1000, /* PwmMin      */
    2000, /* PwmMax      */
    1.0,  /* Roll Scale  */
    1.0,  /* Pitch Scale */
    1.0,  /* Yaw Scale   */
    0.2,  /* Idle Speed  */
    8,    /* Rotor Count */
    {
        /* Rotor # | Roll Scale  |  Pitch Scale  |  Yaw Scale  |  Output Scale */
        /*  0  */  { -0.382683,      0.923880,      -1.000000,     1.000000 },
        /*  1  */  {  0.382683,     -0.923880,      -1.000000,     1.000000 },
        /*  2  */  { -0.923880,      0.382683,       1.000000,     1.000000 },
        /*  3  */  { -0.382683,     -0.923880,       1.000000,     1.000000 },
        /*  4  */  {  0.382683,      0.923880,       1.000000,     1.000000 },
        /*  5  */  {  0.923880,     -0.382683,       1.000000,     1.000000 },
        /*  6  */  {  0.923880,      0.382683,      -1.000000,     1.000000 },
        /*  7  */  { -0.923880,     -0.382683,      -1.000000,     1.000000 }
    }
};

/*
** Local Variables
*/

/*
** Function Prototypes
*/

/*
** Function Definitions
*/

/*=======================================================================================
** End of file ci_config.c
**=====================================================================================*/
    
