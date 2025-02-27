/*---------------------------------------------------------------------------*\
    OneFLOW - LargeScale Multiphysics Scientific Simulation Environment
	Copyright (C) 2017-2019 He Xin and the OneFLOW contributors.
-------------------------------------------------------------------------------
License
    This file is part of OneFLOW.

    OneFLOW is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OneFLOW is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OneFLOW.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "UCom.h"
#include "FaceTopo.h"
#include "BcRecord.h"
#include "UnsGrid.h"
#include "FaceMesh.h"
#include "CellMesh.h"
#include "CellTopo.h"
#include "Zone.h"
#include <iostream>
using namespace std;

BeginNameSpace( ONEFLOW )

UGeom ug;

UGeom::UGeom()
{
    ;
}

UGeom::~UGeom()
{
    ;
}

void UGeom::Init()
{
    UnsGrid * grid = Zone::GetUnsGrid();

    ug.nBFace = grid->nBFace;
    ug.nCell = grid->nCell;
    ug.nTCell = grid->nCell + grid->nBFace;
    ug.nFace = grid->nFace;

    this->SetStEd( F_TOTAL );
	this->CreateBcRegion();

    FaceTopo * faceTopo = grid->faceTopo;
    ug.lcf = & faceTopo->lCell;
    ug.rcf = & faceTopo->rCell;

    FaceMesh * faceMesh = grid->faceMesh;
    CellMesh * cellMesh = grid->cellMesh;
    CellTopo * cellTopo = grid->cellMesh->cellTopo;

    ug.fnx = & faceMesh->xfn;
    ug.fny = & faceMesh->yfn;
    ug.fnz = & faceMesh->zfn;
    ug.fvn = & faceMesh->fvn;
    ug.farea = & faceMesh->area;

    ug.fvx = & faceMesh->fvx;
    ug.fvy = & faceMesh->fvy;
    ug.fvz = & faceMesh->fvz;

    ug.fcx = & faceMesh->xfc;
    ug.fcy = & faceMesh->yfc;
    ug.fcz = & faceMesh->zfc;

    ug.ccx = & cellMesh->xcc;
    ug.ccy = & cellMesh->ycc;
    ug.ccz = & cellMesh->zcc;

    ug.cvol  = & cellMesh->vol;
    ug.cvol1 = & cellMesh->vol;
    ug.cvol2 = & cellMesh->vol;

    ug.blankf = & cellTopo->blank;

    cellTopo->CmpC2f( faceTopo );

    ug.c2f = & cellTopo->c2f;

	//ug.ireconface = 0;
	ug.ireconface = 1;
}

void UGeom::CreateBcRegion()
{
    UnsGrid * grid = Zone::GetUnsGrid();
    BcRecord * bcRecord = grid->faceTopo->bcManager->bcRecord;
    bcRecord->CreateBcRegion();

    ug.bcRecord = bcRecord;
}

void UGeom::SetStEd( int flag )
{
    if ( flag == F_INNER )
    {
        this->ist = 0;
        this->ied = ug.nCell;
    }
    else if ( flag == F_GHOST )
    {
        this->ist = ug.nCell;
        this->ied = ug.nTCell;
    }
    else if ( flag == F_TOTAL )
    {
        this->ist = 0;
        this->ied = ug.nTCell;
    }
}

void UGeom::DumpCellFace( int cId )
{
    int nFace = ( * this->c2f )[ cId ].size();
    for ( int fId = 0; fId < nFace; ++ fId )
    {
        cout << ( * this->c2f )[ cId ][ fId ] << " ";
    }
    cout << endl;
}

void AddF2CField( MRField * cellField, MRField * faceField )
{
    int nEqu = cellField->GetNEqu();
    for ( int fId = 0; fId < ug.nBFace; ++ fId )
    {
        ug.fId = fId;
		ug.lc = ( * ug.lcf )[ ug.fId ];
		ug.rc = ( * ug.rcf )[ ug.fId ];
        //if ( ug.lc == 0 ) cout << fId << endl;

        for ( int iEqu = 0; iEqu < nEqu; ++ iEqu )
        {
			( * cellField )[ iEqu ][ ug.lc ] -= ( * faceField )[ iEqu ][ ug.fId ];
        }
    }

    for ( int fId = ug.nBFace; fId < ug.nFace; ++ fId )
    {
        ug.fId = fId;
		ug.lc = ( * ug.lcf )[ ug.fId ];
		ug.rc = ( * ug.rcf )[ ug.fId ];

        //if ( ug.lc == 0 || ug.rc == 0 ) cout << fId << endl;

        for ( int iEqu = 0; iEqu < nEqu; ++ iEqu )
        {
            ( * cellField )[ iEqu ][ ug.lc ] -= ( * faceField )[ iEqu ][ ug.fId ];
            ( * cellField )[ iEqu ][ ug.rc ] += ( * faceField )[ iEqu ][ ug.fId ];
        }
    }
}

void AddF2CFieldDebug( MRField * cellField, MRField * faceField )
{
    int nEqu = cellField->GetNEqu();
    for ( int fId = 0; fId < ug.nBFace; ++ fId )
    {
        ug.fId = fId;
		ug.lc = ( * ug.lcf )[ ug.fId ];
		ug.rc = ( * ug.rcf )[ ug.fId ];

        for ( int iEqu = 0; iEqu < nEqu; ++ iEqu )
        {
			( * cellField )[ iEqu ][ ug.lc ] -= ( * faceField )[ iEqu ][ ug.fId ];
        }
    }

    for ( int fId = ug.nBFace; fId < ug.nFace; ++ fId )
    {
        ug.fId = fId;
		ug.lc = ( * ug.lcf )[ ug.fId ];
		ug.rc = ( * ug.rcf )[ ug.fId ];

        for ( int iEqu = 0; iEqu < nEqu; ++ iEqu )
        {
            ( * cellField )[ iEqu ][ ug.lc ] -= ( * faceField )[ iEqu ][ ug.fId ];
            ( * cellField )[ iEqu ][ ug.rc ] += ( * faceField )[ iEqu ][ ug.fId ];
        }
    }
}

EndNameSpace