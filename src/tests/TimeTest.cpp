/*
 * Hydrogen
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 * Copyright(c) 2008-2021 The hydrogen development team [hydrogen-devel@lists.sourceforge.net]
 *
 * http://www.hydrogen-music.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see https://www.gnu.org/licenses
 *
 */

#include <core/config.h>

#include "TimeTest.h"
#include <core/CoreActionController.h>
#include <core/AudioEngine/AudioEngine.h>
#include <core/Hydrogen.h>
#include <core/Basics/Song.h>
#include <core/Helpers/Filesystem.h>

#include <cmath>
#include <QTest>

using namespace H2Core;

void TimeTest::setUp(){
	
	m_sValidPath = QString( "%1/hydrogen_time_test.h2song" )
		.arg( QDir::tempPath() );

	// We need a song that has at least the maximum pattern group
	// number provided in testElapsedTime(). An empty one won't do it.
	auto pCoreActionController = Hydrogen::get_instance()->getCoreActionController();
	pCoreActionController->openSong( QString( "%1/GM_kit_demo3.h2song" ).arg( Filesystem::demos_dir() ) );
	pCoreActionController->saveSongAs( m_sValidPath );
	
	pCoreActionController->activateTimeline( true );
	pCoreActionController->addTempoMarker( 0, 120 );
	pCoreActionController->addTempoMarker( 3, 100 );
	pCoreActionController->addTempoMarker( 5, 40 );
	pCoreActionController->addTempoMarker( 7, 200 );
}

void TimeTest::tearDown(){
	
	// Delete all temporary files
	if ( QFile::exists( m_sValidPath ) ) {
		QFile::remove( m_sValidPath );
	}
}

float TimeTest::locateAndLookupTime( int nPatternPos ){
	auto pHydrogen = Hydrogen::get_instance();
	auto pAudioEngine = pHydrogen->getAudioEngine();
	auto pCoreActionController = pHydrogen->getCoreActionController();

	pCoreActionController->locateToColumn( nPatternPos );
	return pAudioEngine->getElapsedTime();
}

void TimeTest::testFrameToTickConversion() {
	auto pHydrogen = Hydrogen::get_instance();
	auto pAudioEngine = pHydrogen->getAudioEngine();

	int nRemainingFrames1, nRemainingFrames2, nRemainingFrames3;

	long aux;
	
	long long nFrame1 = 342732;
	long long nFrame2 = 1037223;
	long long nFrame3 = 453610333722;
	long nTick1 = pAudioEngine->computeTickFromFrame( nFrame1, &nRemainingFrames1 );
	long nTick2 = pAudioEngine->computeTickFromFrame( nFrame2, &nRemainingFrames2 );
	long nTick3 = pAudioEngine->computeTickFromFrame( nFrame3, &nRemainingFrames3 );
	float fTickSize1 =
		AudioEngine::computeTickSize( pHydrogen->getAudioOutput()->getSampleRate(),
									  pAudioEngine->getBpmAtColumn( pHydrogen->getColumnForTick( nTick1, true, &aux ) ),
									  pHydrogen->getSong()->getResolution() );
	float fTickSize2 =
		AudioEngine::computeTickSize( pHydrogen->getAudioOutput()->getSampleRate(),
									  pAudioEngine->getBpmAtColumn( pHydrogen->getColumnForTick( nTick2, true, &aux ) ),
									  pHydrogen->getSong()->getResolution() );
	float fTickSize3 =
		AudioEngine::computeTickSize( pHydrogen->getAudioOutput()->getSampleRate(),
									  pAudioEngine->getBpmAtColumn( pHydrogen->getColumnForTick( nTick3, true, &aux ) ),
									  pHydrogen->getSong()->getResolution() );
	long long nFrame1Computed = pAudioEngine->computeFrameFromTick( nTick1 ) +
			static_cast<long long>(std::floor( fTickSize1 )) -
			static_cast<long long>(nRemainingFrames1);
	long long nFrame2Computed = pAudioEngine->computeFrameFromTick( nTick2 ) +
			static_cast<long long>(std::floor( fTickSize2 )) -
			static_cast<long long>(nRemainingFrames2);
	long long nFrame3Computed = pAudioEngine->computeFrameFromTick( nTick3 ) +
			static_cast<long long>(std::floor( fTickSize3 )) -
			static_cast<long long>(nRemainingFrames3);

	long nTick4 = 552;
	long nTick5 = 1939;
	long nTick6 = 534623409;
	long long nFrame4 = pAudioEngine->computeFrameFromTick( nTick4 );
	long long nFrame5 = pAudioEngine->computeFrameFromTick( nTick5 );
	long long nFrame6 = pAudioEngine->computeFrameFromTick( nTick6 );
	long nTick4Computed = pAudioEngine->computeTickFromFrame( nFrame4, &nRemainingFrames1 );
	long nTick5Computed = pAudioEngine->computeTickFromFrame( nFrame5, &nRemainingFrames2 );
	long nTick6Computed = pAudioEngine->computeTickFromFrame( nFrame6, &nRemainingFrames3 );
	float fTickSize4 =
		AudioEngine::computeTickSize( pHydrogen->getAudioOutput()->getSampleRate(),
									  pAudioEngine->getBpmAtColumn( pHydrogen->getColumnForTick( nTick4, true, &aux ) ),
									  pHydrogen->getSong()->getResolution() );
	float fTickSize5 =
		AudioEngine::computeTickSize( pHydrogen->getAudioOutput()->getSampleRate(),
									  pAudioEngine->getBpmAtColumn( pHydrogen->getColumnForTick( nTick5, true, &aux ) ),
									  pHydrogen->getSong()->getResolution() );
	float fTickSize6 =
		AudioEngine::computeTickSize( pHydrogen->getAudioOutput()->getSampleRate(),
									  pAudioEngine->getBpmAtColumn( pHydrogen->getColumnForTick( nTick6, true, &aux ) ),
									  pHydrogen->getSong()->getResolution() );
	
	// Due to the rounding error in AudioEngine::computeTick() and
	// AudioEngine::computeFrame() a small mismatch is allowed.
	CPPUNIT_ASSERT( abs( nFrame1Computed - nFrame1 ) <= 1 );
	CPPUNIT_ASSERT( abs( nFrame2Computed - nFrame2 ) <= 1 );
	CPPUNIT_ASSERT( abs( nFrame3Computed - nFrame3 ) <= 1 );
	CPPUNIT_ASSERT( abs( nTick4Computed - nTick4 ) <= 1 );
	CPPUNIT_ASSERT( nRemainingFrames1 == std::floor( fTickSize4 ) );
	CPPUNIT_ASSERT( abs( nTick5Computed - nTick5 ) <= 1 );
	CPPUNIT_ASSERT( nRemainingFrames2 == std::floor( fTickSize5 ) );
	CPPUNIT_ASSERT( abs( nTick6Computed - nTick6 ) <= 1 );
	CPPUNIT_ASSERT( nRemainingFrames3 == std::floor( fTickSize6 ) );
}

void TimeTest::testElapsedTime(){

	CPPUNIT_ASSERT( std::abs( locateAndLookupTime( 0 ) - 0 ) < 0.0001 );
	CPPUNIT_ASSERT( std::abs( locateAndLookupTime( 1 ) - 2 ) < 0.0001 );
	CPPUNIT_ASSERT( std::abs( locateAndLookupTime( 2 ) - 4 ) < 0.0001 );
	CPPUNIT_ASSERT( std::abs( locateAndLookupTime( 3 ) - 6 ) < 0.0001 );
	CPPUNIT_ASSERT( std::abs( locateAndLookupTime( 4 ) - 8.4 ) < 0.0001 );
	CPPUNIT_ASSERT( std::abs( locateAndLookupTime( 5 ) - 10.8 ) < 0.0001 );
	CPPUNIT_ASSERT( std::abs( locateAndLookupTime( 6 ) - 16.8 ) < 0.0001 );
	CPPUNIT_ASSERT( std::abs( locateAndLookupTime( 7 ) - 22.8 ) < 0.0001 );
	CPPUNIT_ASSERT( std::abs( locateAndLookupTime( 8 ) - 24 ) < 0.0001 );
	CPPUNIT_ASSERT( std::abs( locateAndLookupTime( 1 ) - 2 ) < 0.0001 );
	CPPUNIT_ASSERT( std::abs( locateAndLookupTime( 5 ) - 10.8 ) < 0.0001 );
	CPPUNIT_ASSERT( std::abs( locateAndLookupTime( 2 ) - 4 ) < 0.0001 );
}
