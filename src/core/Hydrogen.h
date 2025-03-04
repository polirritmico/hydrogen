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
#ifndef HYDROGEN_H
#define HYDROGEN_H

#include <core/config.h>
#include <core/Basics/Song.h>
#include <core/Basics/Sample.h>
#include <core/Object.h>
#include <core/Timeline.h>
#include <core/IO/AudioOutput.h>
#include <core/IO/MidiInput.h>
#include <core/IO/MidiOutput.h>
#include <core/IO/JackAudioDriver.h>
#include <core/Basics/Drumkit.h>
#include <core/CoreActionController.h>
#include <core/Timehelper.h>

#include <stdint.h> // for uint32_t et al
#include <cassert>
#include <memory>

inline int randomValue( int max );

namespace H2Core
{
	class CoreActionController;
	class AudioEngine;
///
/// Hydrogen Audio Engine.
///
/** \ingroup docCore*/
class Hydrogen : public H2Core::Object<Hydrogen>
{
	H2_OBJECT(Hydrogen)
public:
	
	/** Specifies where the #AudioEngine does get its current tempo
		updates from.*/
	enum class Tempo {
		/** BeatCounter, TapTempo, OSC and MIDI commands as well as
			the BPM widget in the PlayerControl are used to change the
			tempo.*/
		Song = 0,
		/** Only tempo markers on the Timeline are considered.*/
		Timeline = 1,
		/** Hydrogen will disregard all internal tempo settings and
			uses the ones provided by the JACK server instead. This
			mode is only used in case the JACK audio driver is used,
			JACK timebase support is activated in the Preferences, and
			an external timebase master is registered to the JACK
			server.*/
		Jack = 2
	};
	/**
	 * Creates all the instances used within Hydrogen in the right
	 * order.
	 */
	static void		create_instance();
	/**
	 * Returns the current Hydrogen instance #__instance.
	 */
	static Hydrogen*	get_instance(){ assert(__instance); return __instance; };

	/*
	 * return central instance of the audio engine
	 */
	AudioEngine*		getAudioEngine() const;

	/**
	 * Destructor taking care of most of the clean up.
	 */
	~Hydrogen();

// ***** SEQUENCER ********
	/// Start the internal sequencer
	void			sequencer_play();

	/// Stop the internal sequencer
	void			sequencer_stop();

	void			midi_noteOn( Note *note );

	///Last received midi message
	QString			m_LastMidiEvent;
	int				m_nLastMidiEventParameter;

	/** Wrapper around AudioEngine::toggleNextPatterns().*/
	void			toggleNextPatterns( int nPatternNumber );
	/** Wrapper around AudioEngine::flushAndAddNextPatterns().*/
	bool			flushAndAddNextPatterns( int nPatternNumber );
	
		/**
		 * Get the current song.
		 * \return #__song
		 */ 	
		std::shared_ptr<Song>			getSong() const{ return __song; }
		/**
		 * Sets the current song #__song to @a newSong.
		 * \param newSong Pointer to the new Song object.
		 */
		void			setSong	( std::shared_ptr<Song> newSong );

	/**
	 * Find a PatternList/column corresponding to the supplied tick
	 * position @a nTick.
	 *
	 * Adds up the lengths of all pattern columns until @a nTick lies in
	 * between the bounds of a Pattern.
	 *
	 * \param nTick Position in ticks.
	 * \param bLoopMode Whether looping is enabled in the Song, see
	 *   Song::is_loop_enabled(). If true, @a nTick is allowed to be
	 *   larger than the total length of the Song.
	 * \param pPatternStartTick Pointer to an integer the beginning of the
	 *   found pattern list will be stored in (in ticks).
	 * \return
	 *   - -1 : pattern list couldn't be found.
	 *   - >=0 : PatternList index in Song::__pattern_group_sequence.
	 */
	int			getColumnForTick( long nTick, bool bLoopMode, long* pPatternStartTick ) const;
	/**
	 * Get the total number of ticks passed up to a @a nColumn /
	 * pattern group.
	 *
	 * The AudioEngine should be LOCKED when calling this!
	 *
	 * \param nColumn pattern group.
	 * \return
	 *  - -1 : if @a nColumn is bigger than the number of patterns in
	 *   the Song and Song::isLoopEnabled() is set to false or
	 *   no Patterns could be found at all.
	 *  - >= 0 : the total number of ticks passed.
	 */
	long			getTickForColumn( int nColumn ) const;
	/**
	 * Get the length (in ticks) of the @a nPattern th pattern.
	 *
	 * \param nPattern Position + 1 of the desired PatternList.
	 * \return 
	 * - __-1__ : if not Song was initialized yet.
	 * - #MAX_NOTES : if @a nPattern was smaller than 1, larger
	 * than the length of the vector of the PatternList in
	 * Song::m_pPatternGroupSequence or no Pattern could be found
	 * in the PatternList at @a nPattern - 1.
	 * - __else__ : length of first Pattern found at @a nPattern.
	 */
	long			getPatternLength( int nPattern ) const;

	Song::Mode getMode() const;
	/** Wrapper around Song::setMode() which also triggers
	EVENT_SONG_MODE_ACTIVATION and should be used by all parts of the
	code except for song reading/setting.*/
	void setMode( Song::Mode mode );
	
	Song::ActionMode getActionMode() const;
	/** Wrapper around Song::setActionMode() which also triggers
	EVENT_ACTION_MODE_CHANGE and should be used by all parts of the
	code except for song reading/setting.*/
	void setActionMode( Song::ActionMode mode );

	Song::PatternMode getPatternMode() const;
	/** Wrapper around Song::setPatternMode() which also triggers
	EVENT_STACKED_MODE_ACTIVATION and should be used by all parts of the
	code except for song reading/setting.*/
	void setPatternMode( Song::PatternMode mode );

	/** Wrapper around both Song::setIsTimelineActivated (recent) and
	Preferences::setUseTimelinebpm() (former place to store the
	variable but kept to maintain backward compatibility) which also
	triggers EVENT_TIMELINE_ACTIVATION.*/
	void setIsTimelineActivated( bool bEnabled );

	void			removeSong();

	void updateSongSize();

		void			addRealtimeNote ( int instrument,
							  float velocity,
							  float fPan = 0.0f,
							  bool noteoff=false,
							  int msg1=0 );

		void			restartDrivers();

		AudioOutput*		getAudioOutput() const;
		MidiInput*		getMidiInput() const;
		MidiOutput*		getMidiOutput() const;

		/** Loads the H2Core::Drumkit provided in \a pDrumkitInfo into
		 * the current session.
		 *
		 * During loading all H2Core::Instrument of the current
		 * drumkit will be replaced by the ones in @a pDrumkitInfo top
		 * to bottom. If the latter contains less instruments, the
		 * superfluous ones will be stripped from the
		 * bottom. Depending on the choice of @a bConditional all
		 * instruments will be strip or just those which do not
		 * contain any notes.
		 *
		 * When under session management (see
		 * NsmClient::m_bUnderSessionManagement) the function will
		 * create a symlink to the loaded H2Core::Drumkit using the
		 * name "drumkit" in the folder
		 * NsmClient::m_sSessionFolderPath.
		 *
		 * \param pDrumkit Full-fledged H2Core::Drumkit to load.
		 * \param bConditional Whether to remove all redundant
		 * H2Core::Instrument regardless of their content.
		 *
		 * \returns 0 on success.
		 */
		int			loadDrumkit( Drumkit* pDrumkit, bool bConditional = true );

		/** Test if an Instrument has some Note in the Pattern (used to
		    test before deleting an Instrument)*/
		bool 			instrumentHasNotes( std::shared_ptr<Instrument> pInst );

		/** Delete an #Instrument.*/
		void			removeInstrument( int nInstrumentNumber );

		/** \return m_sCurrentDrumkitName */
		QString	getCurrentDrumkitName() const;
		/** \param sName sets m_sCurrentDrumkitName */
		void			setCurrentDrumkitName( const QString& sName );
		/** \return m_currentDrumkitLookup */
		Filesystem::Lookup	getCurrentDrumkitLookup() const;
		/** \param lookup sets m_currentDrumkitLookup */
		void			setCurrentDrumkitLookup( Filesystem::Lookup lookup );

		void			raiseError( unsigned nErrorCode );


void			previewSample( Sample *pSample );
	void			previewInstrument( std::shared_ptr<Instrument> pInstr );

	/** Recalculates all Samples using RubberBand for a specific
		tempo @a fBpm.
	*
	* This function requires the calling function to lock the
	* #AudioEngine first.
	*/ 
	void recalculateRubberband( float fBpm );
	/** Wrapper around Song::setIsModified() that checks whether a
		song is set.*/
	void setIsModified( bool bIsModified );
	/** Wrapper around Song::getIsModified() that checks whether a
		song is set.*/
	bool getIsModified() const;

	enum ErrorMessages {
		/**
		 * The provided input string in createDriver() does
		 * not match any of the choices for
		 * Preferences::m_sAudioDriver.
		 */
		UNKNOWN_DRIVER,
		/**
		 * Unable to connect the audio driver stored in
		 * #H2Core::AudioEngine::m_pAudioDriver in
		 * audioEngine_startAudioDrivers(). The NullDriver
		 * will be used as a fallback instead.
		 */
		ERROR_STARTING_DRIVER,
		JACK_SERVER_SHUTDOWN,
		JACK_CANNOT_ACTIVATE_CLIENT,
		/**
		 * Unable to connect either the
		 * JackAudioDriver::output_port_1 and the
		 * JackAudioDriver::output_port_name_1 as well as the
		 * JackAudioDriver::output_port_2 and the
		 * JackAudioDriver::output_port_name_2 port using
		 * _jack_connect()_ (jack/jack.h) or the fallback
		 * version using the first two input ports in
		 * JackAudioDriver::connect().
		 */
		JACK_CANNOT_CONNECT_OUTPUT_PORT,
		/**
		 * The client of Hydrogen can not be disconnected from
		 * the JACK server using _jack_client_close()_
		 * (jack/jack.h). Used within JackAudioDriver::disconnect().
		 */
		JACK_CANNOT_CLOSE_CLIENT,
		/**
		 * Unable to register output ports for the JACK client
		 * using _jack_port_register()_ (jack/jack.h) in
		 * JackAudioDriver::init() or
		 * JackAudioDriver::setTrackOutput().
		 */
		JACK_ERROR_IN_PORT_REGISTER,
		/**
		 * Unable to start the OSC server with the given
		 * port number. 
		 */
		OSC_CANNOT_CONNECT_TO_PORT,
		PLAYBACK_TRACK_INVALID
	};

	void			onTapTempoAccelEvent();
	void			setTapTempo( float fInterval );

	void			restartLadspaFX();
	/** \return #m_nSelectedPatternNumber*/
	int				getSelectedPatternNumber() const;
	/**
	 * Sets #m_nSelectedPatternNumber.
	 *
	 *\param nPat Sets #m_nSelectedPatternNumber
	 * \param bNeedsLock Whether the function was called with the
	 * audio engine locked already or it should do so itself.
	 */
	void			setSelectedPatternNumber( int nPat, bool bNeedsLock = true );

	/**
	 * Updates the selected pattern to the one recorded note will be
	 * inserted to.
	 */
	void updateSelectedPattern( bool bNeedsLock = true );

	int				getSelectedInstrumentNumber() const;
	void			setSelectedInstrumentNumber( int nInstrument );
	std::shared_ptr<Instrument>		getSelectedInstrument() const;

	/**
	 * Calls audioEngine_renameJackPorts() if
	 * Preferences::m_bJackTrackOuts is set to true.
	 * \param pSong Handed to audioEngine_renameJackPorts().
	 */
	void			renameJackPorts(std::shared_ptr<Song> pSong);

	/** Starts/stops the OSC server
	 * \param bEnable `true` = start, `false` = stop.*/
	void			toggleOscServer( bool bEnable );
	/** Destroys and recreates the OscServer singleton in order to
		adopt a new OSC port.*/
	void			recreateOscServer();
	void			startNsmClient();

	// beatconter
	void			setbeatsToCount( int beatstocount);
	int			getbeatsToCount();
	void			setNoteLength( float notelength);
	float			getNoteLength();
	int			getBcStatus();
	bool			handleBeatCounter();
	void			setBcOffsetAdjust();

	/** Calling JackAudioDriver::releaseTimebaseMaster() directly from
	    the GUI*/
	void			offJackMaster();
	/** Calling JackAudioDriver::initTimebaseMaster() directly from
	    the GUI*/
	void			onJackMaster();

	void			__panic();
	std::shared_ptr<Timeline>	getTimeline() const;
	void			setTimeline( std::shared_ptr<Timeline> );
	
	//export management
	bool			getIsExportSessionActive() const;
	/** \return true on success.*/
	bool			startExportSession( int rate, int depth );
	void			stopExportSession();
	void			startExportSong( const QString& filename );
	void			stopExportSong();
	
	CoreActionController* 	getCoreActionController() const;

	/************************************************************/
	/********************** Playback track **********************/
	/**
	 * Wrapper around Song::setPlaybackTrackEnabled().
	 */
	void			mutePlaybackTrack( const bool bMuted );
	/**
	 * Wrapper around Song::getPlaybackTrackState().
	 */
	Song::PlaybackTrack		getPlaybackTrackState() const;
	/**
	 * Wrapper function for loading the playback track.
	 */
	void			loadPlaybackTrack( QString sFilename );
	/************************************************************/

	/** Specifies the state of the Qt GUI*/
	enum class		GUIState {
		/**There is a GUI but it is not ready yet (during startup).*/
		notReady = -1,
		/**No GUI available.*/
		unavailable = 0,
		/**There is a working GUI.*/
		ready = 1
	};
	
	/**\return #m_GUIState*/
	GUIState		getGUIState() const;
	/**\param state Specifies whether the Qt5 GUI is active. Sets
	   #m_GUIState.*/
	void			setGUIState( const GUIState state );
	/**
	 * \return Whether JackAudioDriver is used as current audio
	 * driver.
	 */
	bool			hasJackAudioDriver() const;
	/**
	 * \return Whether JackAudioDriver is used as current audio driver
	 * and JACK transport was activated via the GUI
	 * (#H2Core::Preferences::m_bJackTransportMode).
	 */
	bool			hasJackTransport() const;
        float			getMasterBpm() const;

	/**
	 * Convenience function checking whether using the Timeline tempo
	 * is set in the Preferences, Song::SONG_MODE is set, and there is
	 * a JACK timebase master present.
	 *
	 * \return Whether the Timeline is used to determine the current speed.
	 */
	bool isTimelineEnabled() const;

	/**
	 * Convenience function checking whether using the Pattern Editor
	 * is locked in the song settings and the song is in song mode.
	 */
	bool isPatternEditorLocked() const;
	void setIsPatternEditorLocked( bool bValue );

	Tempo getTempoSource() const;
	
	/**
	 * \return Whether we hasJackTransport() and there is an external
	 * JACK timebase master broadcasting us tempo information and
	 * making use disregard Hydrogen's Timeline information (see
	 * #H2Core::JackAudioDriver::m_timebaseState).
	 */
	JackAudioDriver::Timebase		getJackTimebaseState() const;
	/** \return NsmClient::m_bUnderSessionManagement if NSM is
		supported.*/
	bool			isUnderSessionManagement() const;

	///midi lookuptable
	int 			m_nInstrumentLookupTable[MAX_INSTRUMENTS];

	/**
	 * Returns the #License of the drumkit found in @a sDrumkitPath.
	 *
	 * It caches the results in m_licenseMap.
	 */
	License getLicenseFromDrumkit( const QString& sDrumkitPath );
	void addDrumkitLicenseToCache( const License& license, const QString& sDrumkitPath );

	/**
	 * Add @a pInstr to __instrument_death_row and triggers
	 * __kill_instruments().
	 *
	 * Since there might still be some notes of @a pInstr left in one
	 * of the note queues, the instrument can not be deleted right
	 * away. Instead, this function will add it to a list of
	 * instruments marked for deletion and it will be dealt with at a
	 * later time.
	 */
	void addInstrumentToDeathRow( std::shared_ptr<Instrument> pInstr );
	
	/** Formatted string version for debugging purposes.
	 * \param sPrefix String prefix which will be added in front of
	 * every new line
	 * \param bShort Instead of the whole content of all classes
	 * stored as members just a single unique identifier will be
	 * displayed without line breaks.
	 *
	 * \return String presentation of current object.*/
	QString toQString( const QString& sPrefix, bool bShort = true ) const override;

private:
	/**
	 * Static reference to the Hydrogen singleton. 
	 *
	 * It is created using the Hydrogen::Hydrogen() constructor,
	 * initialized with NULL and assigned a new Hydrogen instance
	 * if still 0 in create_instance().
	 */
	static Hydrogen* 	__instance;

	/**
	 * Pointer to the current song. It is initialized with NULL in
	 * the Hydrogen() constructor, set via setSong(), and accessed
	 * via getSong().
	 */
	std::shared_ptr<Song>			__song;

	/**
	 * Auxiliary function setting a bunch of global variables.
	 *
	 * - #m_ntaktoMeterCompute = 1;
	 * - #m_nbeatsToCount = 4;
	 * - #m_nEventCount = 1;
	 * - #m_nTempoChangeCounter = 0;
	 * - #m_nBeatCount = 1;
	 * - #m_nCoutOffset = 0;
	 * - #m_nStartOffset = 0;
	 */
	void initBeatcounter();

	// beatcounter
	float			m_ntaktoMeterCompute;	///< beatcounter note length
	int			m_nbeatsToCount;	///< beatcounter beats to count
	int			m_nEventCount;		///< beatcounter event
	int			m_nTempoChangeCounter;	///< count tempochanges for timeArray
	int			m_nBeatCount;		///< beatcounter beat to count
	double			m_nBeatDiffs[16];	///< beat diff
	timeval 		m_CurrentTime;		///< timeval
	int			m_nCoutOffset;		///ms default 0
	int			m_nStartOffset;		///ms default 0
	//~ beatcounter


	// used for song export
	Song::Mode		m_oldEngineMode;
	bool			m_bOldLoopEnabled;
	bool			m_bExportSessionIsActive;
	
	/**
	 * Specifies whether the Qt5 GUI is active.
	 *
	 * When a new Song is set via the core part of Hydrogen, e.g. in
	 * the context of session management, the std::shared_ptr<Song> must* be set via
	 * the GUI if active. Else the GUI will freeze.
	 *
	 * Set by setGUIState() and accessed via getGUIState().
	 */
	GUIState		m_GUIState;
	
	/**
	 * Local instance of the Timeline object.
	 */
	std::shared_ptr<Timeline>	m_pTimeline;
	/**
	 * Local instance of the CoreActionController object.
	 */ 
	CoreActionController* 	m_pCoreActionController;
	
	/// Deleting instruments too soon leads to potential crashes.
	std::list<std::shared_ptr<Instrument>> 	__instrument_death_row; 
	
	/**
	 * Instrument currently focused/selected in the GUI. 
	 *
	 * Within the core it is relevant for the MIDI input. Using
	 * Preferences::__playselectedinstrument incoming MIDI signals can be
	 * used to play back only the selected instrument or the whole
	 * drumkit.
	 */
	int				m_nSelectedInstrumentNumber;
	/**
	 * Index of the pattern selected in the GUI or by a MIDI event.
	 */
	int				m_nSelectedPatternNumber;

	/**
	 * Onset of the recorded last in addRealtimeNote(). It is used to
	 * determine the custom length of the note in case the note on
	 * event is followed by a note off event.
	 */
	int				m_nLastRecordedMIDINoteTick;
	/*
	 * Central instance of the audio engine. 
	 */
	AudioEngine*	m_pAudioEngine;

	/**
	 * Map associating drumkit paths with the license found in the
	 * corresponding drumkit.xml file.
	 *
	 * As retrieving a license based on a drumkit name/path is rather
	 * expensive, this object will be used for caching.
	 */
	std::map<QString, License> m_licenseMap;

	/** 
	 * Constructor, entry point, and initialization of the
	 * Hydrogen application.
	 *
	 * It is called by the main() function after setting up a
	 * bunch of Qt5 stuff and creating an instance of the Logger
	 * and Preferences.
	 *
	 * Only one Hydrogen object is allowed to exist. If the
	 * #__instance object is present, the constructor will throw
	 * an error.
	 */
	Hydrogen();

	void __kill_instruments();

};


/*
 * inline methods
 */
inline std::shared_ptr<Timeline> Hydrogen::getTimeline() const
{
	return m_pTimeline;
}
inline void Hydrogen::setTimeline( std::shared_ptr<Timeline> pTimeline )
{
	m_pTimeline = pTimeline;
}

inline CoreActionController* Hydrogen::getCoreActionController() const
{
	return m_pCoreActionController;
}

inline bool Hydrogen::getIsExportSessionActive() const
{
	return m_bExportSessionIsActive;
}

inline AudioEngine* Hydrogen::getAudioEngine() const {
	return m_pAudioEngine;
}

inline Hydrogen::GUIState Hydrogen::getGUIState() const {
	return m_GUIState;
}

inline void Hydrogen::setGUIState( const Hydrogen::GUIState state ) {
	m_GUIState = state;
}
inline int Hydrogen::getSelectedPatternNumber() const
{
	return m_nSelectedPatternNumber;
}
inline int Hydrogen::getSelectedInstrumentNumber() const
{
	return m_nSelectedInstrumentNumber;
}
};

#endif

