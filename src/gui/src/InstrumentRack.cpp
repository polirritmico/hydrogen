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

#include "InstrumentRack.h"
#include "HydrogenApp.h"
#include "CommonStrings.h"
#include "Widgets/Button.h"
#include "InstrumentEditor/InstrumentEditorPanel.h"
#include "SoundLibrary/SoundLibraryPanel.h"
#include "HydrogenApp.h"

#include <QGridLayout>

InstrumentRack::InstrumentRack( QWidget *pParent )
 : QWidget( pParent )
 , Object()
{
	

	auto pPref = H2Core::Preferences::get_instance();
	
	resize( 290, 405 );
	setMinimumSize( width(), height() );
	setFixedWidth( width() );

	QFont fontButtons( H2Core::Preferences::get_instance()->getApplicationFontFamily(), getPointSize( pPref->getFontSize() ) );

// TAB buttons
	QWidget *pTabButtonsPanel = new QWidget( nullptr );
	pTabButtonsPanel->setFixedHeight( 24 );
	pTabButtonsPanel->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

	// instrument editor button
	m_pShowInstrumentEditorBtn = new Button( pTabButtonsPanel, QSize( 145, 24 ), Button::Type::Toggle, "", HydrogenApp::get_instance()->getCommonStrings()->getInstrumentButton(), false, QSize(), tr( "Show Instrument editor" ) );
	connect( m_pShowInstrumentEditorBtn, &QPushButton::clicked,
			 [=]() { m_pSoundLibraryPanel->hide();
				 InstrumentEditorPanel::get_instance()->show();
			 });
	connect( m_pShowInstrumentEditorBtn, &QPushButton::clicked,
			 this, &InstrumentRack::tabButtonClicked );

	// show sound library button
	m_pShowSoundLibraryBtn = new Button( pTabButtonsPanel,QSize( 145, 24 ), Button::Type::Toggle, "", HydrogenApp::get_instance()->getCommonStrings()->getSoundLibraryButton(), false, QSize(), tr( "Show sound library" ) );
	connect( m_pShowSoundLibraryBtn, &QPushButton::clicked,
			 [=]() { m_pSoundLibraryPanel->show();
				 InstrumentEditorPanel::get_instance()->hide();
			 });
	connect( m_pShowSoundLibraryBtn, &QPushButton::clicked,
			 this, &InstrumentRack::tabButtonClicked );

	QHBoxLayout *pTabHBox = new QHBoxLayout();
	pTabHBox->setSpacing( 0 );
	pTabHBox->setMargin( 0 );
	pTabHBox->addWidget( m_pShowInstrumentEditorBtn );
	pTabHBox->addWidget( m_pShowSoundLibraryBtn );

	pTabButtonsPanel->setLayout( pTabHBox );

//~ TAB buttons


	InstrumentEditorPanel::get_instance()->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

	m_pSoundLibraryPanel = new SoundLibraryPanel( nullptr, false );

	// LAYOUT
	QGridLayout *pGrid = new QGridLayout();
	pGrid->setSpacing( 0 );
	pGrid->setMargin( 0 );

	pGrid->addWidget( pTabButtonsPanel, 0, 0, 1, 3 );
	pGrid->addWidget( InstrumentEditorPanel::get_instance(), 2, 1 );
	pGrid->addWidget( m_pSoundLibraryPanel, 2, 1 );

	this->setLayout( pGrid );
	
	connect( HydrogenApp::get_instance(), &HydrogenApp::preferencesChanged, this, &InstrumentRack::onPreferencesChanged );
	
	InstrumentEditorPanel::get_instance()->show();
	m_pSoundLibraryPanel->hide();
	m_pShowInstrumentEditorBtn->setChecked( true );
	m_pShowSoundLibraryBtn->setChecked( false );
}



InstrumentRack::~InstrumentRack()
{
	INFOLOG( "DESTROY" );
}

void InstrumentRack::tabButtonClicked() {
	if ( m_pSoundLibraryPanel->isVisible() ) {
		m_pShowSoundLibraryBtn->setChecked( true );
		m_pShowInstrumentEditorBtn->setChecked( false );
	}
	else if ( InstrumentEditorPanel::get_instance()->isVisible() ) {
		m_pShowSoundLibraryBtn->setChecked( false );
		m_pShowInstrumentEditorBtn->setChecked( true );
	}
	else {
		ERRORLOG( "Neither the sound library panel nor the instrument editor panel are visible" );
	}
}

void InstrumentRack::onPreferencesChanged(  H2Core::Preferences::Changes changes ) {
	auto pPref = H2Core::Preferences::get_instance();
	
	if ( changes & H2Core::Preferences::Changes::Font ) {
		QFont fontButtons( pPref->getApplicationFontFamily(), getPointSize( pPref->getFontSize() ) );
		m_pShowInstrumentEditorBtn->setFont( fontButtons );
		m_pShowSoundLibraryBtn->setFont( fontButtons );
	}
}
