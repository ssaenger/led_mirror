/* ********************************************************************************************
 * animatrix.hpp
 *
 * Author: Shawn Saenger
 *
 * Created: July 22, 2024
 *
 * Description:      ANIMartRIX header file implemented for this project. Modified code from
 *                   Stefan Petrick https://github.com/StefanPetrick/animartrix
 *
 * ********************************************************************************************
 */

#include "../inc/overide.h"
#include "../inc/animations.hpp"

/* --------------------------------------------------------------------------------------------
 *  CONSTANTS
 * --------------------------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------------------------
 * ANIMAX_SD_CS define
 *
 * 
 */
#define ANIMAX_SD_CS 55

/* --------------------------------------------------------------------------------------------
 *  DEFINITIONS
 * --------------------------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------------------------
 * ANIMAX_DIRECTORY define
 *
 * 
 *
 */
#ifndef ANIMAX_DIRECTORY
#define ANIMAX_DIRECTORY "/gifs/"
#endif /* ANIMAX_DIRECTORY */

/* --------------------------------------------------------------------------------------------
 *  PUBLIC FUNCTIONS
 * --------------------------------------------------------------------------------------------
 */


bool ANIMAX_Init();

void ANIMAX_Lava1(AniParms *Ap);

void ANIMAX_ChasingSpirals(AniParms *Ap);

void ANIMAX_Caleido1(AniParms *Ap);

void ANIMAX_Zoom(AniParms *Ap);

void ANIMAX_Rings(AniParms *Ap);

void Animax_HotBlob(AniParms *Ap);

void ANIMAX_Waves(AniParms *Ap);

void ANIMAX_CenterField(AniParms *Ap);

void ANIMAX_Caleido2(AniParms *Ap);

void ANIMAX_Caleido3(AniParms *Ap);

void ANIMAX_Scaledemo1(AniParms *Ap);

void ANIMAX_Yves(AniParms *Ap);

void ANIMAX_Spiralus(AniParms *Ap);

void ANIMAX_Spiralus2(AniParms *Ap);
