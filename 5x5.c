/*

     5X5 - SIMPLE GAME FOR THE USR PILOT
     Copyright (C) 1997,1998,1999 David A Pearson
   
     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the license, or 
     (at your option) any later version.
     
     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.
     
     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

/*
 * Modification history:
 *
 * $Log: 5x5.c,v $
 * Revision 1.2  1999/05/11 12:53:13  davep
 * Fix from Gary Chapman <garychapman@yahoo.com> which fixes a bug when
 * tapping on the very edge of the play area.
 *
 * Revision 1.1  1997/06/21 07:55:05  davep
 * Initial revision
 *
 */

#pragma pack( 2 )

/* Pilot header files. */

#include <Common.h>
#include <System/SysAll.h>
#include <UI/UIAll.h>

/* Local header files. */

#include "5x5.h"

/* Structure for the app prefs. */

struct strPrefs5x5
{
    int iMoves;
    int iOn;
    int playArea[ 5 ][ 5 ];
};

/* Prototype local functions. */

void DrawForm( void );
void DrawBox( int, int, int, int );
void DrawPlayArea( void );
void RefreshPlayArea( void );
void InitGame( int );
void RefreshScores( void );
void YouWin( void );
void NewGame( int );
void MakeMove( EventPtr );
void SaveGameState( void );

/* Global variables. */

static FormPtr  frm;                  /* Pointer to the form. */
static FieldPtr fldMoves;             /* Pointer to the moves field. */
static FieldPtr fldOn;                /* Pointer to the On field. */
static FieldPtr fldOff;               /* Pointer to the Off field. */
static int      playArea[ 5 ][ 5 ];   /* Flags for the play area. */
static int      dirtyCells[ 5 ][ 5 ]; /* Track which cells need drawing */
static int      iOn    = 0;           /* How many cells are filled? */
static int      iMoves = 0;           /* Number of moves made. */
static int      iLastX = -1;          /* X position of last penDown. */
static int      iLastY = -1;          /* Y position of last penDown */
static VoidHand hMoves;               /* Memory handle for moves field. */
static VoidHand hOn;                  /* Memory handle for on field. */
static VoidHand hOff;                 /* Memory handle for off field. */

/* Location of the play area. */

#define PA_TX     35            /* Top X */
#define PA_TY     20            /* Top Y */
#define PA_BX    155            /* Bottom X */
#define PA_BY    140            /* Bottom Y */
#define PA_CELL   24            /* Height/Width of a cell */
    
/* Macro to check that an event was in the play area. */

#define EVENT_IN_PLAYAREA( e ) ( e.screenX > PA_TX && \
                                 e.screenX < PA_BX && \
                                 e.screenY > PA_TY && \
                                 e.screenY < PA_BY )

/* Macro to toggle the state of a cell. */
#define TOGGLE_CELL( x, y )    ( playArea[ x ][ y ] = !playArea[ x ][ y ],\
                                 dirtyCells[ x ][ y ] = 1,\
                                 iOn += ( playArea[ x ][ y ] ? 1 : -1 ) )

/*
 */
    
DWord PilotMain( Word cmd, Ptr cmdPBP, Word launchFlags )
{
    if ( cmd == sysAppLaunchCmdNormalLaunch )
    {
        short err;
        EventType e;
        int iExit = 0;

        FrmGotoForm( ID_Frm5x5 );

        InitGame( 0 );

        hMoves = MemHandleNew( 10 );
        hOn    = MemHandleNew( 10 );
        hOff   = MemHandleNew( 10 );
        
        while ( !iExit )
        {
            EvtGetEvent( &e, -1 );

            if ( SysHandleEvent( &e ) )
            {
                continue;
            }

            if ( MenuHandleEvent( ( void * ) 0, &e, &err ) )
            {
                continue;
            }

            switch ( e.eType )
            {
                case frmLoadEvent :
                    
                    frm = FrmInitForm( e.data.frmLoad.formID );
                    FrmSetActiveForm( frm );
                    
                    fldMoves = FrmGetObjectPtr( frm, FrmGetObjectIndex( frm, ID_FldMoves ) );
                    fldOn    = FrmGetObjectPtr( frm, FrmGetObjectIndex( frm, ID_FldOn ) );
                    fldOff   = FrmGetObjectPtr( frm, FrmGetObjectIndex( frm, ID_FldOff ) );
                    
                    break;
                    
                case frmOpenEvent :
                    
                    DrawForm();
                    RefreshScores();
                    break;
                    
                case penDownEvent :
                    
                    if ( EVENT_IN_PLAYAREA( e ) )
                    {
                        iLastX = ( ( e.screenX - PA_TX ) / PA_CELL );
                        iLastY = ( ( e.screenY - PA_TY ) / PA_CELL );
                    }
                    else
                    {
                        FrmHandleEvent( FrmGetActiveForm(), &e );
                    }
                    break;
                    
                case penUpEvent :
                    
                    if ( EVENT_IN_PLAYAREA( e ) )
                    {
                        MakeMove( &e );
                    }
                    else
                    {
                        FrmHandleEvent( FrmGetActiveForm(), &e );
                    }
                    break;
                    
                case menuEvent :
                    
                    switch ( e.data.menu.itemID )
                    {
                        case ID_MnuItmNew :
                            NewGame( iMoves );
                            break;
                            
                        case ID_MnuItmAbout :
                            FrmAlert( ID_AltAbout );
                            break;
                    }
                    
                    break;
                    
                case ctlSelectEvent :
                    
                    NewGame( iMoves );
                    break;
                    
                case appStopEvent:
                    
                    FrmCloseAllForms();
                    iExit = 1;
                    break;
                    
                default:
                    
                    FrmHandleEvent( FrmGetActiveForm(), &e );
                    break;
            }
        }
        
        /* One would assume that you should free the handle here.
           However, doing this gives an "invalid handle" error.
           If you are reading this and you know what I'm doing wrong
           then please feel free to drop me a line and let me know.
        */
        
        /*MemHandleFree( hMoves );
          MemHandleFree( hOn );
          MemHandleFree( hOff );*/
        
        SaveGameState();
    }
    
    return( 0 );
}

/*
 */

void DrawForm( void )
{
    FrmDrawForm( FrmGetActiveForm() );
    DrawPlayArea();
}

/*
 */

void DrawPlayArea( void )
{
    DrawBox( PA_TX, PA_TY, PA_BX, PA_BY );
    DrawBox( PA_TX + PA_CELL, PA_TY, PA_BX - PA_CELL, PA_BY );
    DrawBox( PA_TX + PA_CELL + PA_CELL, PA_TY,
             PA_BX - PA_CELL - PA_CELL, PA_BY );
    DrawBox( PA_TX, PA_TY + PA_CELL, PA_BX, PA_BY - PA_CELL );
    DrawBox( PA_TX, PA_TY + PA_CELL + PA_CELL, PA_BX,
             PA_BY - PA_CELL - PA_CELL);

    RefreshPlayArea();
}

/*
 */

void RefreshPlayArea( void )
{
    RectangleType r;
    int x, y;

    for ( x = 0; x < 5; x++ )
    {
        for ( y = 0; y < 5; y++ )
        {
            if ( dirtyCells[ x ][ y ] )
            {
                r.topLeft.x = PA_TX + ( PA_CELL * x ) + 4;
                r.topLeft.y = PA_TY + ( PA_CELL * y ) + 4;
                r.extent.x  = PA_CELL - 8;
                r.extent.y  = PA_CELL - 8;
                
                if ( playArea[ x ][ y ] )
                {
                    WinDrawRectangle( &r, 8 );
                }
                else
                {
                    WinEraseRectangle( &r, 8 );
                }
                
                dirtyCells[ x ][ y ] = 0;
            }
        }
    }
}

/*
 */

void DrawBox( int x1, int y1, int x2, int y2 )
{
    WinDrawLine( x1, y1, x1, y2 );
    WinDrawLine( x1, y1, x2, y1 );
    WinDrawLine( x2, y1, x2, y2 );
    WinDrawLine( x1, y2, x2, y2 );
}

/*
 */

void InitGame( int iNew )
{
    struct strPrefs5x5 prefs;
    
    if ( iNew || !PrefGetAppPreferencesV10( (ULong) 'DAP0', 1, (VoidPtr) &prefs,
                                            sizeof( prefs ) ) )
    {
        int x, y;

        iMoves = 0;
        iOn    = 0;
        
        for ( x = 0; x < 5; x++ )
        {
            for ( y = 0; y < 5; y++ )
            {
                playArea[ x ][ y ]   = 0;
                dirtyCells[ x ][ y ] = 1;
            }
        }
        
        TOGGLE_CELL( 1, 2 );
        TOGGLE_CELL( 2, 1 );
        TOGGLE_CELL( 2, 2 );
        TOGGLE_CELL( 2, 3 );
        TOGGLE_CELL( 3, 2 );
    }
    else
    {
        int x, y;
        
        iMoves = prefs.iMoves;
        iOn    = prefs.iOn;

        for ( x = 0; x < 5; x++ )
        {
            for ( y = 0; y < 5; y++ )
            {
                playArea[ x ][ y ]   = prefs.playArea[ x ][ y ];
                dirtyCells[ x ][ y ] = 1;
            }
        }
    }
}

/*
 */

void RefreshScores( void )
{
    CharPtr sMoves = (CharPtr) MemHandleLock( hMoves );
    CharPtr sOn    = (CharPtr) MemHandleLock( hOn );
    CharPtr sOff   = (CharPtr) MemHandleLock( hOff );

    StrIToA( sMoves, iMoves   );
    StrIToA( sOn,    iOn      );
    StrIToA( sOff,   25 - iOn );
    
    MemHandleUnlock( hMoves );
    MemHandleUnlock( hOn );
    MemHandleUnlock( hOff );
    
    FldSetTextHandle( fldMoves, (Handle) hMoves );
    FldSetTextHandle( fldOn,    (Handle) hOn    );
    FldSetTextHandle( fldOff,   (Handle) hOff   );
    
    FldDrawField( fldMoves );
    FldDrawField( fldOn    );
    FldDrawField( fldOff   );
}

/*
 */

void YouWin( void )
{
    FrmAlert( ID_AltWin );
    NewGame( 0 );
}

/*
 */

void NewGame( int iAsk )
{
    if ( ( iAsk ? FrmAlert( ID_AltNew ) == 0 : 1 ) )
    {
        InitGame( 1 );
        RefreshPlayArea();
        RefreshScores();
    }
}

/*
 */

void MakeMove( EventPtr e )
{
    int x = ( ( e->screenX - PA_TX ) / PA_CELL );
    int y = ( ( e->screenY - PA_TY ) / PA_CELL );
    
    if ( x == iLastX && y == iLastY )
    {
        TOGGLE_CELL( x, y );
        
        if ( x > 0 )
        {
            TOGGLE_CELL( x - 1, y );
        }
        if ( x < 4 )
        {
            TOGGLE_CELL( x + 1, y );
        }
        if ( y > 0 )
        {
            TOGGLE_CELL( x, y - 1 );
        }
        if ( y < 4 )
        {
            TOGGLE_CELL( x, y + 1 );
        }

        ++iMoves;

        RefreshPlayArea();
        RefreshScores();
        
        if ( iOn == 25 )
        {
            YouWin();
        }
    }
}

/*
 */

void SaveGameState( void )
{
    struct strPrefs5x5 prefs;
    int x, y;

    prefs.iMoves = iMoves;
    prefs.iOn    = iOn;
    
    for ( x = 0; x < 5; x++ )
    {
        for ( y = 0; y < 5; y++ )
        {
            prefs.playArea[ x ][ y ] = playArea[ x ][ y ];
        }
    }

    PrefSetAppPreferencesV10( 'DAP0', 1, &prefs, sizeof( prefs ) );
}
