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
#ifndef ACTION_H
#define ACTION_H
#include <core/Object.h>
#include <map>
#include <memory>
#include <string>
#include <cassert>

/** \ingroup docCore docMIDI */
class Action : public H2Core::Object<Action> {
	H2_OBJECT(Action)
	public:
	Action( QString sType = "NOTHING" );

		void setParameter1( QString text ){
			m_sParameter1 = text;
		}

		void setParameter2( QString text ){
			m_sParameter2 = text;
		}

		void setParameter3( QString text ){
			m_sParameter3 = text;
		}

		void setValue( QString text ){
			m_sValue = text;
		}

		QString getParameter1() const {
			return m_sParameter1;
		}

		QString getParameter2() const {
			return m_sParameter2;
		}

		QString getParameter3() const {
			return m_sParameter3;
		}

		QString getValue() const {
			return m_sValue;
		}

		QString getType() const {
			return m_sType;
		}

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
		QString m_sType;
		QString m_sParameter1;
		QString m_sParameter2;
		QString m_sParameter3;
		QString m_sValue;
};

namespace H2Core
{
	class Hydrogen;
}

/** \ingroup docCore docMIDI */
class MidiActionManager : public H2Core::Object<MidiActionManager>
{
	H2_OBJECT(MidiActionManager)
	private:
		/**
		 * Object holding the current MidiActionManager
		 * singleton. It is initialized with NULL, set with
		 * create_instance(), and accessed with
		 * get_instance().
		 */
		static MidiActionManager *__instance;

		/**
		 * Holds the names of all Action identifiers which Hydrogen is
		 * able to interpret.
		 */
	QStringList m_actionList;

		typedef bool (MidiActionManager::*action_f)(std::shared_ptr<Action> , H2Core::Hydrogen * );
		/**
		 * Holds all Action identifiers which Hydrogen is able to
		 * interpret.  
		 *
		 * It holds a pair consisting of a pointer to member function
		 * performing the desired action and an integer specifying how
		 * many additional Action parameters are required to do so.
		 */
	std::map<QString, std::pair<action_f,int>> m_actionMap;
		bool play(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool play_stop_pause_toggle(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool stop(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool pause(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool record_ready(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool record_strobe_toggle(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool record_strobe(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool record_exit(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool mute(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool unmute(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool mute_toggle(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool strip_mute_toggle(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool strip_solo_toggle(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool next_bar(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool previous_bar(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool bpm_increase(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool bpm_decrease(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool bpm_cc_relative(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool bpm_fine_cc_relative(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool master_volume_relative(std::shared_ptr<Action> , H2Core::Hydrogen *);
		bool master_volume_absolute(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool strip_volume_relative(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool strip_volume_absolute(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool effect_level_relative(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool effect_level_absolute(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool select_next_pattern(std::shared_ptr<Action> , H2Core::Hydrogen * );
	bool select_only_next_pattern(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool select_next_pattern_cc_absolute(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool select_next_pattern_promptly(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool select_next_pattern_relative(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool select_and_play_pattern(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool pan_relative(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool pan_absolute(std::shared_ptr<Action> , H2Core::Hydrogen * );
	bool pan_absolute_sym(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool filter_cutoff_level_absolute(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool beatcounter(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool tap_tempo(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool playlist_song(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool playlist_next_song(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool playlist_previous_song(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool toggle_metronome(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool select_instrument(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool undo_action(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool redo_action(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool gain_level_absolute(std::shared_ptr<Action> , H2Core::Hydrogen * );
		bool pitch_level_absolute(std::shared_ptr<Action> , H2Core::Hydrogen * );

		QStringList m_eventList;

		int m_nLastBpmChangeCCParameter;

	bool setSong( int nSongNumber, H2Core::Hydrogen* pHydrogen );

	public:

		/**
		 * Handles multiple actions at once and calls handleAction()
		 * on them.
		 *
		 * \return true - in case all actions were successful, false - otherwise.
		 */
	bool handleActions( std::vector<std::shared_ptr<Action>> );
		/**
		 * The handleAction method is the heart of the
		 * MidiActionManager class. It executes the operations that
		 * are needed to carry the desired action.
		 */
		bool handleAction( std::shared_ptr<Action> );
		/**
		 * If #__instance equals 0, a new MidiActionManager
		 * singleton will be created and stored in it.
		 *
		 * It is called in H2Core::Hydrogen::create_instance().
		 */
		static void create_instance();
		/**
		 * Returns a pointer to the current MidiActionManager
		 * singleton stored in #__instance.
		 */
		static MidiActionManager* get_instance() { assert(__instance); return __instance; }

		QStringList getActionList(){
			return m_actionList;
		}

		QStringList getEventList(){
			return m_eventList;
		}
	/**
	 * \return -1 in case the @a couldn't be found.
	 */
	int getParameterNumber( const QString& sActionType ) const;

		MidiActionManager();
		~MidiActionManager();
};
#endif
