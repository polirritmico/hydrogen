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

#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <core/Object.h>
#include <core/Basics/Note.h>
#include <cassert>
#include <mutex>

/** Maximum number of events to be stored in the
    H2Core::EventQueue::__events_buffer.*/
#define MAX_EVENTS 1024

namespace H2Core
{
	
/** Basic types of communication between the core part of Hydrogen and
    its GUI.*/
enum EventType {
	/** Fallback event*/
	EVENT_NONE,
	EVENT_STATE,
	/** 
	 * The list of currently played patterns in changed.
	 *
	 * In #Song::Mode::Song this is triggered every time the column of
	 * the SongEditor grid changed. Either by rolling transport or by
	 * relocation.
	 *
	 * It is handled by EventListener::patternChangedEvent().
	 */
	EVENT_PATTERN_CHANGED,
	/**
	 * A pattern was added, deleted, or modified.
	 */
	EVENT_PATTERN_MODIFIED,
	/** Another pattern was selected via MIDI or the GUI without
	 * affecting the audio transport. While the selection in the
	 * former case already happens in the GUI, this event will be used
	 * to tell it the selection was successful and had been done.
	 *
	 * Handled by EventListener::selectedPatternChangedEvent().
	 */
	EVENT_SELECTED_PATTERN_CHANGED,
	EVENT_SELECTED_INSTRUMENT_CHANGED,
	/** Some parameters of an instrument have been changed.
	 *
	 * Numbers `>=0` indicate the number of the instrument that has been
	 * changed. `-1` indicates that multiple instruments were altered.
	 */
	EVENT_INSTRUMENT_PARAMETERS_CHANGED,
	EVENT_MIDI_ACTIVITY,
	EVENT_XRUN,
	EVENT_NOTEON,
	EVENT_ERROR,
	/** Event indicating the triggering of the
	 * #H2Core::AudioEngine::m_pMetronomeInstrument.
	 *
	 * In AudioEngine::updateNoteQueue() the pushing of this Event is
	 * decoupled from the creation and queuing of the corresponding
	 * Note itself.
	 *
	 * In Director it triggers a change in the displayed column
	 * number, tempo, and tag.
	 *
	 * The associated values do correspond to the following actions:
	 * - 0: Beat at the beginning of a Pattern in
	 *      audioEngine_updateNoteQueue(). The corresponding Note will
	 *      be created with a pitch of 3 and velocity of 1.0.
	 *      Sets MetronomeWidget::m_state to
	 *      MetronomeWidget::METRO_ON and triggers
	 *      MetronomeWidget::updateWidget().
	 * - 1: Beat in the remainder of a Pattern in
	 *      audioEngine_updateNoteQueue(). The corresponding Note will
	 *      be created with a pitch of 0 and velocity of 0.8.
	 *      Sets MetronomeWidget::m_state to
	 *      MetronomeWidget::METRO_FIRST and triggers
	 *      MetronomeWidget::updateWidget().
	 *
	 * Handled by EventListener::metronomeEvent().
	 */
	EVENT_METRONOME,
	EVENT_PROGRESS,
	EVENT_JACK_SESSION,
	EVENT_PLAYLIST_LOADSONG,
	EVENT_UNDO_REDO,
	EVENT_SONG_MODIFIED,
	EVENT_TEMPO_CHANGED,
	/**
	 * Event triggering the loading or saving of the
	 * H2Core::Preferences whenever they were changed outside of the
	 * GUI, e.g. by session management or an OSC command.
	 *
	 * If the value of the event is 
	 * - 0 - tells the GUI to save the current geometry settings in
	 * the H2Core::Preferences file.
	 * - 1 - tells the GUI to load the Preferences file and to  update
	 * a bunch of widgets, checkboxes etc. to reflect the changes in
	 * the configuration.
	 */
	EVENT_UPDATE_PREFERENCES,
	/**
	 * Event triggering HydrogenApp::updateSongEvent() whenever the
	 * Song was changed outside of the GUI, e.g. by session management
	 * or and OSC command.
	 *
	 * If the value of the event is 
	 * - 0 - update the GUI to represent the song loaded by the core.
	 * - 1 - triggered whenever the Song was saved via the core part
	 *       (updated the title and status bar).
	 * - 2 - Song is not writable (inform the user via a QMessageBox)
	 */
	EVENT_UPDATE_SONG,
	/**
	 * Triggering HydrogenApp::quitEvent() and enables a shutdown of
	 * the entire application via the command line.
	 */
	EVENT_QUIT,

	/** Enables/disables the usage of the Timeline.*/ 
	EVENT_TIMELINE_ACTIVATION,
	/** Tells the GUI some parts of the Timeline (tempo markers or
		tags) were modified.*/
	EVENT_TIMELINE_UPDATE,
	/** Toggles the button indicating the usage Jack transport.*/
	EVENT_JACK_TRANSPORT_ACTIVATION,
	/** Toggles the button indicating the usage Jack timebase master
		and informs the GUI about a state change.*/
	EVENT_JACK_TIMEBASE_STATE_CHANGED,
	EVENT_SONG_MODE_ACTIVATION,
	/** Song::PatternMode::Stacked (0) or Song::PatternMode::Selected
		(1) was activated */
	EVENT_STACKED_MODE_ACTIVATION,
	/** Toggles the button indicating the usage loop mode.*/
	EVENT_LOOP_MODE_ACTIVATION,
	/** Switches between select mode (0) and draw mode (1) in the *SongEditor.*/
	EVENT_ACTION_MODE_CHANGE,
	EVENT_GRID_CELL_TOGGLED,
	/** Triggered when transport is moved into a different column
		(either during playback or when relocated by the user)*/
	EVENT_COLUMN_CHANGED,
	/** A the current drumkit was replaced by a new one*/
	EVENT_DRUMKIT_LOADED,
	/** Locks the PatternEditor on the pattern currently played back.*/
	EVENT_PATTERN_EDITOR_LOCKED,
	/** Triggered in case there is a relocation of the transport
	 * position due to an user interaction or an incoming
	 * MIDI/OSC/JACK command.
	 */
	EVENT_RELOCATION,
	EVENT_SONG_SIZE_CHANGED,
	EVENT_DRIVER_CHANGED,
	EVENT_PLAYBACK_TRACK_CHANGED
};

/** Basic building block for the communication between the core of
 * Hydrogen and its GUI.  The individual Events will be enlisted in
 * the EventQueue singleton.*/
/** \ingroup docCore docEvent */
class Event
{
public:
	/** Specifies the context the event is create in and which
	    function should be triggered to handle it.*/
	EventType type;
	/** Additional information to describe the actual context of
	    the engine.*/
	int value;
};

/** Object handling the communication between the core of Hydrogen and
 * its GUI.
 *
 * Whenever a specific condition is met or occasion happens within the
 * core part of Hydrogen (its engine), an Event will be added to the
 * EventQueue singleton. The GUI checks the content of this queue on a
 * regular basis using HydrogenApp::onEventQueueTimer(). The actual
 * frequency is set in the constructor HydrogenApp::HydrogenApp() to
 * 20 times per second. Now, whenever an Event of a certain EventType
 * is encountered, the corresponding function in the EventListener
 * will be invoked to respond to the condition of the engine. For
 * details about the mapping of EventTypes to functions please see the
 * documentation of HydrogenApp::onEventQueueTimer().*/
/** \ingroup docCore docEvent */
class EventQueue : public H2Core::Object<EventQueue>
{
	H2_OBJECT(EventQueue)
public:/**
	* If #__instance equals 0, a new EventQueue singleton will be
	 * created and stored in it.
	 *
	 * It is called in Hydrogen::create_instance().
	 */
	static void create_instance();
	/**
	 * Returns a pointer to the current EventQueue singleton
	 * stored in #__instance.
	 */
	static EventQueue* get_instance() { assert(__instance); return __instance; }
	~EventQueue();

	/**
	 * Queues the next event into the EventQueue.
	 *
	 * The event itself will be constructed inside the function
	 * and will be two properties: an EventType @a type and a
	 * value @a nValue. Since the event written to the queue most
	 * recently is indexed with #__write_index, this variable is
	 * incremented once and its modulo with respect to #MAX_EVENTS
	 * is calculated to determine the position of insertion into
	 * #__events_buffer.
	 *
	 * The modulo operation is necessary because #__write_index
	 * will be only incremented and does not respect the actual
	 * length of #__events_buffer itself.
	 *
	 * \param type Type of the event, which will be queued.
	 * \param nValue Value specifying the content of the new event.
	 */
	void push_event( const EventType type, const int nValue );
	/**
	 * Reads out the next event of the EventQueue.
	 *
	 * Since the event read out most recently is indexed with
	 * #__read_index, this variable is incremented once and its
	 * modulo with respect to #MAX_EVENTS is calculated to
	 * determine the event returned from #__events_buffer. 
	 *
	 * The modulo operation is necessary because #__read_index
	 * will be only incremented and does not respect the actual
	 * length of #__events_buffer itself.
	 *
	 * \return Next event in line.
	 */
	Event pop_event();

	struct AddMidiNoteVector {
		int m_column;       //position
		int m_row;          //instrument row
		int m_pattern;      // pattern number
		int m_length;
		float f_velocity;
		float f_pan;
		Note::Key nk_noteKeyVal;
		Note::Octave no_octaveKeyVal;
		bool b_isMidi;
		bool b_isInstrumentMode;
	};
	std::vector<AddMidiNoteVector> m_addMidiNoteVector;

	bool getSilent() const;
	void setSilent( bool bSilent );

private:
	/**
	 * Constructor of the EventQueue class.
	 *
	 * It fills all #MAX_EVENTS slots of the #__events_buffer with
	 * #H2Core::EVENT_NONE and assigns itself to #__instance. Called by
	 * create_instance().
	 */
	EventQueue();
	/**
	 * Object holding the current EventQueue singleton. It is
	 * initialized with nullptr, set in EventQueue(), and
	 * accessed via get_instance().
	 */
	static EventQueue *__instance;

	/**
	 * Continuously growing number indexing the event, which has
	 * been read from the EventQueue most recently.
	 *
	 * It is incremented with each call to pop_event(). 
	 */
	volatile unsigned int __read_index;
	/**
	 * Continuously growing number indexing the event, which has
	 * been written to the EventQueue most recently.
	 *
	 * It is incremented with each call to push_event(). 
	 */
	volatile unsigned int __write_index;
	/**
	 * Array of all events contained in the EventQueue.
	 *
	 * Its length is set to #MAX_EVENTS and it gets initialized
	 * with #H2Core::EVENT_NONE in EventQueue().
	 */
	Event __events_buffer[ MAX_EVENTS ];

	/**
	 * Mutex to lock access to queue.
	 */
	std::mutex m_mutex;

	/** Whether or not to push log messages.*/
	bool m_bSilent;
};

inline bool EventQueue::getSilent() const {
	return m_bSilent;
}
inline void EventQueue::setSilent( bool bSilent ) {
	m_bSilent = bSilent;
}

};

#endif
