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

#include "SoundLibraryPanel.h"

#include <QtGui>
#include <QtWidgets>

#include "SoundLibraryDatastructures.h"
#include "SoundLibraryTree.h"
#include "FileBrowser.h"

#include "SoundLibrarySaveDialog.h"
#include "SoundLibraryPropertiesDialog.h"
#include "SoundLibraryExportDialog.h"

#include "../HydrogenApp.h"
#include "../CommonStrings.h"
#include "../Widgets/Button.h"
#include "../Widgets/PixmapWidget.h"
#include "../SongEditor/SongEditorPanel.h"
#include "../PatternEditor/PatternEditorPanel.h"
#include "../PatternEditor/DrumPatternEditor.h"
#include "../PatternEditor/PatternEditorInstrumentList.h"
#include "../InstrumentRack.h"
#include "../InstrumentEditor/InstrumentEditorPanel.h"

#include <core/Basics/Adsr.h>
#include <core/AudioEngine/AudioEngine.h>
#include <core/H2Exception.h>
#include <core/Hydrogen.h>
#include <core/Basics/Instrument.h>
#include <core/Basics/InstrumentList.h>
#include <core/Basics/Pattern.h>
#include <core/Basics/PatternList.h>
#include <core/Basics/Sample.h>
#include <core/Basics/Song.h>
#include <core/Helpers/Filesystem.h>

using namespace H2Core;

#include <cassert>

SoundLibraryPanel::SoundLibraryPanel( QWidget *pParent, bool bInItsOwnDialog )
 : QWidget( pParent )
 , __sound_library_tree( nullptr )
 , __drumkit_menu( nullptr )
 , __instrument_menu( nullptr )
 , __song_menu( nullptr )
 , __pattern_menu( nullptr )
 , __pattern_menu_list( nullptr )
 , __system_drumkits_item( nullptr )
 , __user_drumkits_item( nullptr )
 , __song_item( nullptr )
 , __pattern_item( nullptr )
 , __pattern_item_list( nullptr )
 , m_bInItsOwnDialog( bInItsOwnDialog )
{
	
	//
	__drumkit_menu = new QMenu( this );
	__drumkit_menu->addAction( tr( "Load" ), this, SLOT( on_drumkitLoadAction() ) );
	__drumkit_menu->addAction( tr( "Export" ), this, SLOT( on_drumkitExportAction() ) );
	__drumkit_menu->addAction( tr( "Properties" ), this, SLOT( on_drumkitPropertiesAction() ) );
	__drumkit_menu->addSeparator();
	__drumkit_menu->addAction( tr( "Delete" ), this, SLOT( on_drumkitDeleteAction() ) );

	__instrument_menu = new QMenu( this );
	__instrument_menu->addSeparator();
	__instrument_menu->addAction( tr( "Delete" ), this, SLOT( on_instrumentDeleteAction() ) );

	__song_menu = new QMenu( this );
	__song_menu->addSeparator();
	__song_menu->addAction( tr( "Load" ), this, SLOT( on_songLoadAction() ) );

	__pattern_menu = new QMenu( this );
	__pattern_menu->addSeparator();
	__pattern_menu->addAction( tr( "Load" ), this, SLOT( on_patternLoadAction() ) );
	__pattern_menu->addAction( tr( "Delete" ), this, SLOT( on_patternDeleteAction() ) );

	__pattern_menu_list = new QMenu( this );
	__pattern_menu_list->addSeparator();
	__pattern_menu_list->addAction( tr( "Load" ), this, SLOT( on_patternLoadAction() ) );

// DRUMKIT LIST
	__sound_library_tree = new SoundLibraryTree( nullptr );
	connect( __sound_library_tree, SIGNAL( currentItemChanged ( QTreeWidgetItem*, QTreeWidgetItem* ) ), this, SLOT( on_DrumkitList_ItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ) );
	connect( __sound_library_tree, SIGNAL( itemActivated ( QTreeWidgetItem*, int ) ), this, SLOT( on_DrumkitList_itemActivated( QTreeWidgetItem*, int ) ) );
	connect( __sound_library_tree, SIGNAL( leftClicked(QPoint) ), this, SLOT( on_DrumkitList_leftClicked(QPoint)) );
	if( ! m_bInItsOwnDialog ) {
		connect( __sound_library_tree, SIGNAL( rightClicked(QPoint) ), this, SLOT( on_DrumkitList_rightClicked(QPoint)) );
		connect( __sound_library_tree, SIGNAL( onMouseMove( QMouseEvent* ) ), this, SLOT( on_DrumkitList_mouseMove( QMouseEvent* ) ) );
	}


	// LAYOUT
	QVBoxLayout *pVBox = new QVBoxLayout();
	pVBox->setSpacing( 0 );
	pVBox->setMargin( 0 );

	pVBox->addWidget( __sound_library_tree );
	

	this->setLayout( pVBox );

	__expand_pattern_list = Preferences::get_instance()->__expandPatternItem;
	__expand_songs_list = Preferences::get_instance()->__expandSongItem;

	connect( HydrogenApp::get_instance(), &HydrogenApp::preferencesChanged, this, &SoundLibraryPanel::onPreferencesChanged );
	
	updateDrumkitList();
	
	HydrogenApp::get_instance()->addEventListener(this);
}



SoundLibraryPanel::~SoundLibraryPanel()
{
	if ( auto pH2App = HydrogenApp::get_instance() ) {
		pH2App->removeEventListener( this );
	}

	for (uint i = 0; i < __system_drumkit_info_list.size(); ++i ) {
		delete __system_drumkit_info_list[i];
	}
	__system_drumkit_info_list.clear();

	for (uint i = 0; i < __user_drumkit_info_list.size(); ++i ) {
		delete __user_drumkit_info_list[i];
	}
	__user_drumkit_info_list.clear();

}



void SoundLibraryPanel::updateDrumkitList()
{

	auto pPref = H2Core::Preferences::get_instance();

	__sound_library_tree->clear();

	QFont boldFont( Preferences::get_instance()->getApplicationFontFamily(), getPointSize( pPref->getFontSize() ) );
	boldFont.setBold( true );

	QFont childFont( Preferences::get_instance()->getLevel2FontFamily(), getPointSize( pPref->getFontSize() ) );
	setFont( childFont );

	__system_drumkits_item = new QTreeWidgetItem( __sound_library_tree );
	__system_drumkits_item->setText( 0, tr( "System drumkits" ) );
	__system_drumkits_item->setExpanded( true );
	__system_drumkits_item->setFont( 0, boldFont );

	__user_drumkits_item = new QTreeWidgetItem( __sound_library_tree );
	__user_drumkits_item->setText( 0, tr( "User drumkits" ) );
	__user_drumkits_item->setExpanded( true );
	__user_drumkits_item->setFont( 0, boldFont );
	

	for (uint i = 0; i < __system_drumkit_info_list.size(); ++i ) {
		delete __system_drumkit_info_list[i];
	}
	__system_drumkit_info_list.clear();

	for (uint i = 0; i < __user_drumkit_info_list.size(); ++i ) {
		delete __user_drumkit_info_list[i];
	}
	__user_drumkit_info_list.clear();

	//User drumkit list
	QStringList usr_dks = Filesystem::usr_drumkit_list();
	for (int i = 0; i < usr_dks.size(); ++i) {
		QString absPath = Filesystem::usr_drumkits_dir() + usr_dks[i];
		Drumkit *pInfo = Drumkit::load( absPath, false );
		if (pInfo) {
			__user_drumkit_info_list.push_back( pInfo );
			QTreeWidgetItem* pDrumkitItem = new QTreeWidgetItem( __user_drumkits_item );
			pDrumkitItem->setText( 0, pInfo->get_name() );
			if ( ! m_bInItsOwnDialog ) {
				InstrumentList *pInstrList = pInfo->get_instruments();
				for ( uint nInstr = 0; nInstr < pInstrList->size(); ++nInstr ) {
					auto pInstr = pInstrList->get( nInstr );
					QTreeWidgetItem* pInstrumentItem = new QTreeWidgetItem( pDrumkitItem );
					pInstrumentItem->setText( 0, QString( "[%1] " ).arg( nInstr + 1 ) + pInstr->get_name() );
					pInstrumentItem->setToolTip( 0, pInstr->get_name() );
				}
			}
		}
	}

	//System drumkit list
	QStringList sys_dks = Filesystem::sys_drumkit_list();
	for (int i = 0; i < sys_dks.size(); ++i) {
		QString absPath = Filesystem::sys_drumkits_dir() + sys_dks[i];
		Drumkit *pInfo = Drumkit::load( absPath, false );
		if (pInfo) {
			__system_drumkit_info_list.push_back( pInfo );
			QTreeWidgetItem* pDrumkitItem = new QTreeWidgetItem( __system_drumkits_item );
			pDrumkitItem->setText( 0, pInfo->get_name() );
			if ( ! m_bInItsOwnDialog ) {
				InstrumentList *pInstrList = pInfo->get_instruments();
				for ( uint nInstr = 0; nInstr < pInstrList->size(); ++nInstr ) {
					auto pInstr = pInstrList->get( nInstr );
					QTreeWidgetItem* pInstrumentItem = new QTreeWidgetItem( pDrumkitItem );
					pInstrumentItem->setText( 0, QString( "[%1] " ).arg( nInstr + 1 ) + pInstr->get_name() );
					pInstrumentItem->setToolTip( 0, pInstr->get_name() );
				}
			}
		}
	}

	if ( ! m_bInItsOwnDialog ) {
		//Songlist
		QStringList songs = Filesystem::song_list_cleared();
		if ( songs.size() > 0 ) {
			__song_item = new QTreeWidgetItem( __sound_library_tree );
			__song_item->setText( 0, tr( "Songs" ) );
			__song_item->setToolTip( 0, tr("Double click to expand the list") );
			__song_item->setExpanded( __expand_songs_list );
			__song_item->setFont( 0, boldFont );
			for (uint i = 0; i < songs.size(); i++) {
				QTreeWidgetItem* pSongItem = new QTreeWidgetItem( __song_item );
				QString song = songs[i];
				pSongItem->setText( 0 , song.left( song.indexOf(".")) );
				pSongItem->setToolTip( 0, song );
			}
		}


		//Pattern list
		QStringList patternDirList = Filesystem::pattern_drumkits();
		if ( patternDirList.size() > 0 ) {
		
			__pattern_item = new QTreeWidgetItem( __sound_library_tree );
			__pattern_item->setText( 0, tr( "Patterns" ) );
			__pattern_item->setToolTip( 0, tr("Double click to expand the list") );
			__pattern_item->setExpanded( __expand_pattern_list );
			__pattern_item->setFont( 0, boldFont );
		
			SoundLibraryDatabase* db = SoundLibraryDatabase::get_instance();
			soundLibraryInfoVector* allPatternDirList = db->getAllPatterns();
			QStringList allCategoryNameList = db->getAllPatternCategories();

			//now sorting via category

			for (uint i = 0; i < allCategoryNameList.size(); ++i) {
				QString categoryName = allCategoryNameList[i];

				QTreeWidgetItem* pCategoryItem = new QTreeWidgetItem( __pattern_item );
				pCategoryItem->setText( 0, categoryName  );

				soundLibraryInfoVector::iterator mapIterator;
				for( mapIterator=allPatternDirList->begin(); mapIterator != allPatternDirList->end(); mapIterator++ )
					{
						QString patternCategory = (*mapIterator)->getCategory();
						if ( (patternCategory == categoryName) || (patternCategory.isEmpty() && categoryName == "No category") ){
							QTreeWidgetItem* pPatternItem = new QTreeWidgetItem( pCategoryItem );
							pPatternItem->setText( 0, (*mapIterator)->getName());
							pPatternItem->setText( 1, (*mapIterator)->getPath() );
							pPatternItem->setToolTip( 0, Pattern::loadDrumkitNameFrom( (*mapIterator)->getPath() ));
							INFOLOG( "Path" +  (*mapIterator)->getPath() );
						}
					}
			}
		}
	}
	
	update_background_color();
}



void SoundLibraryPanel::on_DrumkitList_ItemChanged( QTreeWidgetItem * current, QTreeWidgetItem * previous )
{
	UNUSED( previous );
	
	if( current == nullptr ){
		return;
	}

	if ( current->parent() == __system_drumkits_item ||
		 current->parent() == __user_drumkits_item  ){
			emit item_changed( true );
	} else {
		emit item_changed( false );
	}
	
	test_expandedItems();
}



void SoundLibraryPanel::on_DrumkitList_itemActivated( QTreeWidgetItem * item, int column )
{
	UNUSED( column );

//	INFOLOG( "[on_DrumkitList_itemActivated]" );
	if ( item == __system_drumkits_item ||
		 item == __user_drumkits_item ||
		 item == __system_drumkits_item->parent() ||
		 item->parent() == __song_item ||
		 item == __song_item ||
		 item == __pattern_item ||
		 item->parent() == __pattern_item ||
		 item->parent()->parent() == __pattern_item ||
		 item == __pattern_item_list ||
		 item->parent() == __pattern_item_list ||
		 item->parent()->parent() == __pattern_item_list ) {
		return;
	}

	if ( item->parent() == __system_drumkits_item ||
		 item->parent() == __user_drumkits_item  ) {
		// e' stato selezionato un drumkit
	}
	else {

		// e' stato selezionato uno strumento
		QString selectedName = item->text(0);
		if( item->text(0) == "Patterns" ){
			return;
		}

		QString sInstrName = selectedName.remove( 0, selectedName.indexOf( "] " ) + 2 );
		QString sDrumkitName = item->parent()->text(0);
		INFOLOG( QString(sDrumkitName) + ", instr:" + sInstrName );

		auto pInstrument = Instrument::load_instrument( sDrumkitName, sInstrName );
		pInstrument->set_muted( false );

		Hydrogen::get_instance()->getAudioEngine()->getSampler()->preview_instrument( pInstrument );
	}
}







void SoundLibraryPanel::on_DrumkitList_rightClicked( QPoint pos )
{
	if( __sound_library_tree->currentItem() == nullptr ) {
		return;
	}
	
	if (
		( __sound_library_tree->currentItem()->parent() == nullptr ) ||
		( __sound_library_tree->currentItem() == __user_drumkits_item ) ||
		( __sound_library_tree->currentItem() == __system_drumkits_item )
	) {
		return;
	}

	if ( __sound_library_tree->currentItem()->parent() == __song_item ) {
		__song_menu->popup( pos );
	}

	if ( __sound_library_tree->currentItem()->parent()->parent() == __pattern_item && __pattern_item != nullptr ) {
		__pattern_menu->popup( pos );
	}

	if ( __sound_library_tree->currentItem()->parent() == __user_drumkits_item ) {
		__drumkit_menu->popup( pos );
	}
	else if ( __sound_library_tree->currentItem()->parent()->parent() == __user_drumkits_item ) {
		__instrument_menu->popup( pos );
	}
	//else if ( __sound_library_tree->currentItem()->parent()->parent()->parent() ==  __pattern_item_list ) {
	//	__pattern_menu_list->popup( pos );
	//}
	

	if ( __sound_library_tree->currentItem()->parent() == __system_drumkits_item ) {
		__drumkit_menu->popup( pos );
	}
	else if ( __sound_library_tree->currentItem()->parent()->parent() == __system_drumkits_item ) {
		__instrument_menu->popup( pos );
	}
}



void SoundLibraryPanel::on_DrumkitList_leftClicked( QPoint pos )
{
	__start_drag_position = pos;
}



void SoundLibraryPanel::on_DrumkitList_mouseMove( QMouseEvent *event)
{
	if (! ( event->buttons() & Qt::LeftButton ) ) {
		return;
	}

	if ( ( event->pos() - __start_drag_position ).manhattanLength() < QApplication::startDragDistance() ) {
		return;
	}
	
	if ( !__sound_library_tree->currentItem() ) {
		return;
	}

	if (
		( __sound_library_tree->currentItem()->parent() == __system_drumkits_item ) ||
		( __sound_library_tree->currentItem()->parent() == __user_drumkits_item )
	) {
 		// drumkit selection
		//INFOLOG( "ho selezionato un drumkit (system)" );
		return;
	}
	else {
		//INFOLOG( "ho selezionato uno strumento" );
		// instrument selection
		if ( __sound_library_tree->currentItem() == nullptr )
		{
			return;
		}
		
		if ( __sound_library_tree->currentItem()->parent() == nullptr )
		{
			return;
		}

		if ( __sound_library_tree->currentItem()->parent() == __song_item )
		{
			return;
		}

		if ( __sound_library_tree->currentItem()->parent()->text(0) == nullptr )
		{
			return;
		}

		if ( __sound_library_tree->currentItem()->parent() == __pattern_item ) {
			return;
		}

		if ( __sound_library_tree->currentItem()->parent()->parent() == __pattern_item ) {

			QString sPatternPath = __sound_library_tree->currentItem()->text( 1 );
			QString dragtype = "drag pattern";
			QString sText = dragtype + "::" + sPatternPath;

			QDrag *pDrag = new QDrag(this);
			QMimeData *pMimeData = new QMimeData;

			pMimeData->setText( sText );
			pDrag->setMimeData( pMimeData);
			pDrag->exec( Qt::CopyAction | Qt::MoveAction );
			return;
		}

		QString sDrumkitName = __sound_library_tree->currentItem()->parent()->text(0);
		QString sInstrumentName = ( __sound_library_tree->currentItem()->text(0) ).remove( 0, __sound_library_tree->currentItem()->text(0).indexOf( "] " ) + 2 );
		QString sDrumkitScope;
		if ( __sound_library_tree->currentItem()->parent()->parent() == __system_drumkits_item ) {
			sDrumkitScope = "system";
		} else {
			sDrumkitScope = "user";
		}

		QString sText = "importInstrument:" + sDrumkitScope + "::" +
			sDrumkitName + "::" + sInstrumentName;

		QDrag *pDrag = new QDrag(this);
		QMimeData *pMimeData = new QMimeData;

		pMimeData->setText( sText );
		pDrag->setMimeData( pMimeData);

		pDrag->exec( Qt::CopyAction | Qt::MoveAction );
	}
}



void SoundLibraryPanel::on_drumkitLoadAction()
{
	auto pHydrogen = H2Core::Hydrogen::get_instance();
	
	QString sDrumkitName = __sound_library_tree->currentItem()->text(0);
	// Whether we deal with a system or a user drumkit.
	QString sDrumkitType = __sound_library_tree->currentItem()->parent()->text(0);

	Drumkit *pDrumkitInfo = nullptr;

	// Find the drumkit in the list. If the drumkit was listed as a
	// "System drumkit", it won't be searched in the user ones and
	// vice versa.
	if ( sDrumkitType == __system_drumkits_item->text(0) ) {
		
		for ( uint i = 0; i < __system_drumkit_info_list.size(); i++ ) {
			Drumkit *pInfo = __system_drumkit_info_list[i];
			if ( pInfo->get_name() == sDrumkitName ) {
				pDrumkitInfo = pInfo;
				break;
			}
		}
	} else if ( sDrumkitType == __user_drumkits_item->text(0) ) {
		
		for ( uint i = 0; i < __user_drumkit_info_list.size(); i++ ) {
			Drumkit *pInfo = __user_drumkit_info_list[i];
			if ( pInfo->get_name() == sDrumkitName ) {
				pDrumkitInfo = pInfo;
				break;
			}
		}
	} else {
		ERRORLOG( QString( "Unknown drumkit type [%1] for drumkit [%2]" )
				  .arg( sDrumkitType ).arg( sDrumkitName ) );
		return;
	}
	
	if( !pDrumkitInfo ) {
		ERRORLOG( QString( "Unable to find drumkit [%1]" ).arg( sDrumkitName ) );
		return;
	}

	InstrumentList *pSongInstrList = pHydrogen->getSong()->getInstrumentList();
	InstrumentList *pDrumkitInstrList = pDrumkitInfo->get_instruments();

	int oldCount = pSongInstrList->size();
	int newCount = pDrumkitInstrList->size();

	bool conditionalLoad = false;
	bool hasNotes = false;

	INFOLOG("Old kit has " + QString::number( oldCount ) + " instruments, new one has " + QString::number( newCount ) );

	if ( newCount < oldCount )
	{
		// Check if any of the instruments that will be removed have notes
		for ( int i = 0; i < pSongInstrList->size(); i++)
		{
			if ( i >= newCount )
			{
				INFOLOG("Checking if Instrument " + QString::number( i ) + " has notes..." );

				if ( pHydrogen->instrumentHasNotes( pSongInstrList->get( i ) ) )
				{
					hasNotes = true;
					INFOLOG("Instrument " + QString::number( i ) + " has notes" );
				}
			}

		}
	
		if ( hasNotes ) {
			auto pCommonStrings = HydrogenApp::get_instance()->getCommonStrings();
			QMessageBox msgBox;
			msgBox.setWindowTitle("Hydrogen");
			msgBox.setIcon( QMessageBox::Warning );
			msgBox.setText( tr( "The existing kit has %1 instruments but the new one only has %2.\nThe first %2 instruments will be replaced with the new instruments and will keep their notes, but some of the remaining instruments have notes.\nWould you like to keep or discard the remaining instruments and notes?\n").arg( QString::number( oldCount ),QString::number( newCount ) ) );

			msgBox.setStandardButtons( QMessageBox::Save | QMessageBox::Discard |
									   QMessageBox::Cancel );
			msgBox.setButtonText(QMessageBox::Save, tr("Keep"));
			msgBox.setButtonText(QMessageBox::Discard,
								 pCommonStrings->getButtonDiscard() );
			msgBox.setButtonText(QMessageBox::Cancel,
								 pCommonStrings->getButtonCancel());
			msgBox.setDefaultButton(QMessageBox::Cancel);
			
			switch ( msgBox.exec() )
			{
				case QMessageBox::Save:
					// Save old instruments with notes
					conditionalLoad = true;
					break;

				case QMessageBox::Discard:
					// discard extra instruments
					conditionalLoad = false;
					break;

				case QMessageBox::Cancel:
					// Cancel
					return;
			}
		}
	}

	assert( pDrumkitInfo );

	QApplication::setOverrideCursor(Qt::WaitCursor);

	pHydrogen->getCoreActionController()->loadDrumkit( pDrumkitInfo, conditionalLoad );

	QApplication::restoreOverrideCursor();
}

void SoundLibraryPanel::drumkitLoadedEvent() {
	update_background_color();
}

void SoundLibraryPanel::selectedInstrumentChangedEvent() {
	update_background_color();
}

void SoundLibraryPanel::update_background_color()
{
	restore_background_color();
	change_background_color();
}



void SoundLibraryPanel::restore_background_color()
{
	for (int i = 0; i < __system_drumkits_item->childCount() ; i++){
		( __system_drumkits_item->child( i ) )->setBackground( 0, QBrush() );		
	}

	for (int i = 0; i < __user_drumkits_item->childCount() ; i++){
		( __user_drumkits_item->child( i ) )->setBackground(0, QBrush() );
	}
}



void SoundLibraryPanel::change_background_color()
{
	auto pSelectedInstrument = Hydrogen::get_instance()->getSelectedInstrument();
	QString sDrumkitName = pSelectedInstrument->get_drumkit_name();
	Filesystem::Lookup lookup = pSelectedInstrument->get_drumkit_lookup();

	if ( lookup == Filesystem::Lookup::system ||
		 lookup == Filesystem::Lookup::stacked ) {
		for (int i = 0; i < __system_drumkits_item->childCount() ; i++){
			if ( ( __system_drumkits_item->child( i ) )->text( 0 ) == sDrumkitName ){
				( __system_drumkits_item->child( i ) )->setBackground( 0, QColor( 50, 50, 50)  );
				return;
			}
		}
	}
	if ( lookup == Filesystem::Lookup::user ||
		 lookup == Filesystem::Lookup::stacked ) {
		for (int i = 0; i < __user_drumkits_item->childCount() ; i++){
			if ( ( __user_drumkits_item->child( i ))->text( 0 ) == sDrumkitName ){
				( __user_drumkits_item->child( i ) )->setBackground( 0, QColor( 50, 50, 50)  );
				break;
			}
		}
	}
}


void SoundLibraryPanel::on_drumkitDeleteAction()
{
	QTreeWidgetItem* pItem = __sound_library_tree->currentItem();
	QString sDrumkitName = pItem->text(0);
	auto pCommonStrings = HydrogenApp::get_instance()->getCommonStrings();

	Filesystem::Lookup lookup;
	if ( pItem->parent() == __system_drumkits_item ) {
		lookup = Filesystem::Lookup::system;
	} else {
		lookup = Filesystem::Lookup::user;
	}

	// If we delete the current loaded drumkit we can get trouble with some empty pointers
	if ( pItem->text(0) == Hydrogen::get_instance()->getCurrentDrumkitName() &&
		 lookup == Hydrogen::get_instance()->getCurrentDrumkitLookup() ){
		QMessageBox::warning( this, "Hydrogen", tr( "It is not possible to delete the currently loaded drumkit: \n  \"%1\".\nTo delete this drumkit first load another drumkit.").arg(sDrumkitName) );
		return;
	}

	if ( lookup == Filesystem::Lookup::system ) {
		QMessageBox::warning( this, "Hydrogen", QString( "\"%1\" " )
							  .arg(sDrumkitName)
							  .append( tr( "is a system drumkit and can't be deleted.") ) );
		return;
	}

	int res = QMessageBox::warning( this, "Hydrogen",
									tr( "Warning, the \"%1\" drumkit will be deleted from disk.\nAre you sure?").arg(sDrumkitName),
									pCommonStrings->getButtonOk(),
									pCommonStrings->getButtonCancel(),
									nullptr, 1 );
	if ( res == 1 ) {
		return;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);
	bool success = Drumkit::remove( pItem->text(0), lookup );
	test_expandedItems();
	updateDrumkitList();
	QApplication::restoreOverrideCursor();
	if ( !success) {
		QMessageBox::warning( this, "Hydrogen", tr( "Drumkit deletion failed.") );
	}
}



void SoundLibraryPanel::on_drumkitExportAction()
{
	QString sDrumkitName = __sound_library_tree->currentItem()->text(0);
	Filesystem::Lookup lookup;
	if ( __sound_library_tree->currentItem()->parent()->text(0) == __system_drumkits_item->text(0) ) {
		lookup = Filesystem::Lookup::system;
	} else {
		lookup = Filesystem::Lookup::user;
	}
	SoundLibraryExportDialog exportDialog( this, sDrumkitName, lookup );
	exportDialog.exec();
}



void SoundLibraryPanel::on_drumkitPropertiesAction()
{
	QString sDrumkitName = __sound_library_tree->currentItem()->text(0);
	// Whether we deal with a system or a user drumkit.
	QString sDrumkitType = __sound_library_tree->currentItem()->parent()->text(0);

	Drumkit* pDrumkitInfo = nullptr;

	// Find the selected drumkit in the drumkit tree. If it was listed
	// as a "System drumkit", it won't be searched in the user ones
	// and vice versa.
	if ( sDrumkitType == __system_drumkits_item->text(0) ) {
		
		for ( uint i = 0; i < __system_drumkit_info_list.size(); i++ ) {
			Drumkit *pInfo = __system_drumkit_info_list[i];
			if ( pInfo->get_name() == sDrumkitName ) {
				pDrumkitInfo = pInfo;
				break;
			}
		}
	} else if ( sDrumkitType == __user_drumkits_item->text(0) ) {
		
		for ( uint i = 0; i < __user_drumkit_info_list.size(); i++ ) {
			Drumkit*pInfo = __user_drumkit_info_list[i];
			if ( pInfo->get_name() == sDrumkitName ) {
				pDrumkitInfo = pInfo;
				break;
			}
		}
	} else {
		ERRORLOG( QString( "Unknown drumkit type [%1] for drumkit [%2]" )
				  .arg( sDrumkitType ).arg( sDrumkitName ) );
		return;
	}
	
	if( ! pDrumkitInfo ) {
		ERRORLOG( QString( "Unable to find drumkit [%1]" ).arg( sDrumkitName ) );
		return;
	}

	QString sPreDrumkitName = Hydrogen::get_instance()->getCurrentDrumkitName();

	Drumkit* pPreDrumkitInfo = nullptr;

	// Find the currently loaded drumkit in the drumkit tree and use
	// the current lookup to decide whether to search in the system or
	// the user folder.
	if ( Hydrogen::get_instance()->getCurrentDrumkitLookup() == Filesystem::Lookup::system ) {
		for ( uint i = 0; i < __system_drumkit_info_list.size(); i++ ) {
			Drumkit *prInfo = __system_drumkit_info_list[i];
			if ( prInfo->get_name() == sPreDrumkitName ) {
				pPreDrumkitInfo = prInfo;
				break;
			}
		}
	} else {
		for ( uint i = 0; i < __user_drumkit_info_list.size(); i++ ) {
			Drumkit *prInfo = __user_drumkit_info_list[i];
			if ( prInfo->get_name() == sPreDrumkitName ) {
				pPreDrumkitInfo = prInfo;
				break;
			}
		}
	}

	if ( pPreDrumkitInfo == nullptr ){
		QMessageBox::warning( this, "Hydrogen", QString( "%1 [%2]")
							  .arg( HydrogenApp::get_instance()->getCommonStrings()->getSoundLibraryFailedPreDrumkitLoad() )
							  .arg(sPreDrumkitName) );
		return;
	}
	assert( pPreDrumkitInfo );
	
	//open the soundlibrary save dialog
	SoundLibraryPropertiesDialog dialog( this, pDrumkitInfo, pPreDrumkitInfo, false );
	dialog.exec();
}



void SoundLibraryPanel::on_instrumentDeleteAction()
{
	QMessageBox::warning( this, "Hydrogen", tr( "Not implemented yet.") );
	ERRORLOG( "[on_instrumentDeleteAction] not implemented yet" );
}

void SoundLibraryPanel::on_songLoadAction()
{
	QString sFilename = Filesystem::song_path( __sound_library_tree->currentItem()->text( 0 ) );

	HydrogenApp::get_instance()->openSong( sFilename );
}



void SoundLibraryPanel::on_patternLoadAction() {

	QString sPatternName = __sound_library_tree->currentItem()->text( 0 );
	QString sDrumkitName = __sound_library_tree->currentItem()->toolTip ( 0 );
	Hydrogen::get_instance()->getCoreActionController()->openPattern( Filesystem::pattern_path( sDrumkitName,
																								sPatternName ) );
}


void SoundLibraryPanel::on_patternDeleteAction()
{
	auto pCommonStrings = HydrogenApp::get_instance()->getCommonStrings();
	QString patternPath = __sound_library_tree->currentItem()->text( 1 );

	int res = QMessageBox::information( this, "Hydrogen",
										tr( "Warning, the selected pattern will be deleted from disk.\nAre you sure?"),
										pCommonStrings->getButtonOk(),
										pCommonStrings->getButtonCancel(),
										nullptr, 1 );
	if ( res == 1 ) {
		return;
	}

	QFile rmfile( patternPath );
	bool err = rmfile.remove();
	if ( err == false ) {
		ERRORLOG( "Error removing the pattern" );
	}

	SoundLibraryDatabase::get_instance()->updatePatterns();
	test_expandedItems();
	updateDrumkitList();
}


void SoundLibraryPanel::test_expandedItems()
{
	assert( __sound_library_tree );
	if ( __song_item == nullptr) {
		__expand_songs_list = false;
	} else {
		__expand_songs_list = __song_item->isExpanded();
	}
	if ( __pattern_item == nullptr) {
		__expand_pattern_list = false;
	} else {
		__expand_pattern_list = __pattern_item->isExpanded();
	}
	Preferences::get_instance()->__expandSongItem = __expand_songs_list;
	Preferences::get_instance()->__expandPatternItem = __expand_pattern_list;
	//ERRORLOG( QString("songs %1 patterns %2").arg(__expand_songs_list).arg(__expand_pattern_list) );
}

void SoundLibraryPanel::onPreferencesChanged( H2Core::Preferences::Changes changes ) {
	auto pPref = H2Core::Preferences::get_instance();
	
	if ( __system_drumkits_item->child( 0 ) != nullptr &&
		 ( changes & H2Core::Preferences::Changes::Font ) ) {
		
		QFont font( pPref->getLevel2FontFamily(), getPointSize( pPref->getFontSize() ) );
		QFont boldFont( pPref->getApplicationFontFamily(), getPointSize( pPref->getFontSize() ) );
		boldFont.setBold( true );

		int ii, jj;
		QTreeWidgetItem* childNode;
		__system_drumkits_item->setFont( 0, boldFont );
		for ( ii = 0; ii < __system_drumkits_item->childCount(); ii++ ){ 
			childNode = __system_drumkits_item->child( ii );
			childNode->setFont( 0, font );
			for ( jj = 0; jj < childNode->childCount(); jj++ ) {
				childNode->child( jj )->setFont( 0, font );
			}
		}
		__user_drumkits_item->setFont( 0, boldFont );
		for ( ii = 0; ii < __user_drumkits_item->childCount(); ii++ ){ 
			childNode = __user_drumkits_item->child( ii );
			childNode->setFont( 0, font );
			for ( jj = 0; jj < childNode->childCount(); jj++ ) {
				childNode->child( jj )->setFont( 0, font );
			}
		}

		if ( __song_item != nullptr ) {
			__song_item->setFont( 0, boldFont );
			for ( ii = 0; ii < __song_item->childCount(); ii++ ){ 
				__song_item->child( ii )->setFont( 0, font );
				__song_item->setFont( ii, font );
			}
		}

		if ( __pattern_item != nullptr ) {
			__pattern_item->setFont( 0, boldFont );
			for ( ii = 0; ii < __pattern_item->childCount(); ii++ ){ 
				childNode = __pattern_item->child( ii );
				childNode->setFont( 0, font );
				for ( jj = 0; jj < childNode->childCount(); jj++ ) {
					childNode->child( jj )->setFont( 0, font );
				}
			}
		}
	}
}

void SoundLibraryPanel::updateSongEvent( int nValue ) {
	// A new song got loaded
	if ( nValue == 0 ) {
		update_background_color();
	}
}
