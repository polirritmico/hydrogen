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

#ifndef H2C_FILESYSTEM_H
#define H2C_FILESYSTEM_H

#include <core/Object.h>
#include <QtCore/QString>

namespace H2Core
{

	/**
	 * Filesystem is a thin layer over QDir, QFile and QFileInfo
	 */
	/** \ingroup docCore*/
	class Filesystem : public H2Core::Object<Filesystem>
	{
		H2_OBJECT(Filesystem)
		public:
		/** flags available for check_permissions() */
		enum file_perms {
						 is_dir =0x01,
						 is_file=0x02,
						 is_readable=0x04,
						 is_writable=0x08,
						 is_executable=0x10
		};
		
	/** Whenever a drumkit is loaded by name a collision between a
	 * user and a system drumkit carrying the same name can
	 * occur.
	 */
	enum class Lookup {
		/** First, looks in the system drumkits and, afterwards, in
		 * the user drumkits. In case both sets contain a member
		 * sharing the requested name, the user one will override the
		 * system one.
		 *
		 * This is the way Hydrogen <= 1.1 were handling all look ups.
		 */
		stacked = 0,
		/** Only search the user drumkits.*/
		user = 1,
		/** Only search the system drumkits.*/
		system = 2
	};

		static const QString songs_ext;
		static const QString scripts_ext;
		static const QString patterns_ext;
		static const QString playlist_ext;
		static const QString drumkit_ext;
		static const QString themes_ext;
		static const QString songs_filter_name;
		static const QString themes_filter_name;
		static const QString scripts_filter_name;
		static const QString patterns_filter_name;
		static const QString playlists_filter_name;

		/**
		 * check user and system filesystem usability
		 * \param logger is a pointer to the logger instance which will be used
		 * \param sys_path an alternate system data path
		 */
		static bool bootstrap( Logger* logger, const QString& sys_path=nullptr );

		/** returns system data path */
		static QString sys_data_path();
		/** returns user data path */
		static QString usr_data_path();

		/** returns user ladspa paths */
		static QStringList ladspa_paths();

		/** returns system config path */
		static QString sys_config_path();
		/** returns user config path */
		static QString usr_config_path();
		/** returns system empty sample file path */
		static QString empty_sample_path();
		/**
		 * Provides the full path to the current empty song.
		 *
		 * The basename consists of a fixed expression and an optional
		 * suffix ensuring the path does not point to an existing
		 * file.
		 *
		 * Empty songs are handled in Hydrogen as follows: Upon
		 * creation, the empty song will be assigned a
		 * Song::m_sFilename identical to the return value of this
		 * function. This triggers autosave files to be generated
		 * corresponding to empty song path. If the user attempts to
		 * save the song from within the GUI, she will be prompted a
		 * "Save As" dialog and is asked to provide a new name. This
		 * way a file using the empty song path does normally not
		 * exists. But since the return value of this function is
		 * reproducible, Hydrogen is able to recover unsaved
		 * modifications applied to an empty song. If the user - by
		 * design or coincidence - picks the empty song path to save a
		 * file or if the OSC API is used to save the empty song,
		 * empty_song_path() will use a suffix to return yet again a
		 * path to a non-existing file and allow for the behavior
		 * described above.
		 */
		static QString empty_song_path();
		/** Default option to offer the user when saving an empty song
			to disk.*/
		static QString default_song_name();
		/** returns untitled song name */
		static QString untitled_song_name();
		/** Returns a string containing the path to the
		    _click.wav_ file used in the metronome. 
			*
			* It is a concatenation of #__sys_data_path and
			* #CLICK_SAMPLE.
			*/
		static QString click_file_path();
		/** returns click file path from user directory if exists, otherwise from system */
		static QString usr_click_file_path();
		/** returns the path to the drumkit XSD (xml schema definition) file */
		static QString drumkit_xsd_path( );
		/** returns the path to the pattern XSD (xml schema definition) file */
		static QString pattern_xsd_path( );
		/** returns the path to the playlist pattern XSD (xml schema definition) file */
		static QString playlist_xsd_path( );
		/** returns the full path (including filename) of the logfile */
		static QString log_file_path();

		/** returns gui image path */
		static QString img_dir();
		/** returns documentation path */
		static QString doc_dir();
		/** returns internationalization path */
		static QString i18n_dir();
		/** returns user scripts path */
		static QString scripts_dir();
		/** returns user songs path */
		static QString songs_dir();
		/** returns user song path, add file extension */
		static QString song_path( const QString& sg_name );
		/** returns user patterns path */
		static QString patterns_dir();
		/** returns user patterns path for a specific drumkit */
		static QString patterns_dir( const QString& dk_name );
		/** returns user patterns path, add file extension*/
		static QString pattern_path( const QString& dk_name, const QString& p_name );
		/** returns user plugins path */
		static QString plugins_dir();
		/** returns system drumkits path */
		static QString sys_drumkits_dir();
		/** returns user drumkits path */
		static QString usr_drumkits_dir();
		/** returns user playlist path */
		static QString playlists_dir();
		/** returns user playlist path, add file extension */
		static QString playlist_path( const QString& pl_name );
		/** returns untitled playlist file name */
		static QString untitled_playlist_file_name();
		/** returns user cache path */
		static QString cache_dir();
		/** returns user repository cache path */
		static QString repositories_cache_dir();
		/** returns system demos path */
		static QString demos_dir();
		/** returns system xsd path */
		static QString xsd_dir();
		/** returns temp path */
		static QString tmp_dir();
		static QString usr_theme_dir();
		static QString sys_theme_dir();
		/**
		 * touch a temporary file under tmp_dir() and return it's path.
		 * if base has a suffix it will be preserved, spaces will be replaced by underscores.
		 * \param base part of the path
		 */
		static QString tmp_file_path( const QString& base );

		/* DRUMKIT */
		/** Returns the basename if the given path is under an existing user or system drumkit path, otherwise the given fname */
		static QString prepare_sample_path( const QString& fname );
		/** Checks if the given filepath is under an existing user or system drumkit path, not the existence of the file */
		static bool file_is_under_drumkit( const QString& fname);
		/** Returns the index of the basename if the given path is under an existing user or system drumkit path, otherwise -1 */
		static int get_basename_idx_under_drumkit( const QString& fname);
		/** returns list of usable system drumkits ( see Filesystem::drumkit_list ) */
		static QStringList sys_drumkit_list( );
		/** returns list of usable user drumkits ( see Filesystem::drumkit_list ) */
		static QStringList usr_drumkit_list( );
		/**
		 * returns true if the drumkit exists within usable system or user drumkits
		 * \param dk_name the drumkit name
		 */
		static bool drumkit_exists( const QString& dk_name );
		/**
		 * returns path for a drumkit within user drumkit path
		 * \param dk_name the drumkit name
		 */
		static QString drumkit_usr_path( const QString& dk_name );
		static QString drumkit_default_kit();
		/** Returns the path to a H2Core::Drumkit folder.
		 *
		 * The search will first be performed within user-level
		 * drumkits system drumkits using usr_drumkit_list() and
		 * usr_drumkits_dir() and later, in case the H2Core::Drumkit
		 * could not be found, within the system-level drumkits using
		 * sys_drumkit_list() and sys_drumkits_dir().
		 *
		 * When under session management (see
		 * NsmClient::m_bUnderSessionManagement) the function will
		 * first look for a "drumkit" symlink or folder within
		 * NsmClient::m_sSessionFolderPath. If it either is not a
		 * valid H2Core::Drumkit or the not the one corresponding to
		 * \a dk_name, the user- and system-level drumkits will be
		 * searched instead.
		 *
		 * \param dk_name Name of the H2Core::Drumkit. In the
		 *   user-level and system-level lookup it has to correspond
		 *   to the name of the folder holding the samples and the
		 *   #DRUMKIT_XML file. For the usage of a local
		 *   H2Core::Drumkit under session management it has to match
		 *   the second-level "name" node within the
		 *   #DRUMKIT_XML file.
		 * \param lookup Where to search (system/user folder or both)
		 * for the drumkit.
		 * \param bSilent whether the function should trigger log
		 *   messages. If set to true, the calling function is
		 *   expected to handle the log messages instead.
		 *
		 * \returns Full path to the folder containing the samples of
		 *   the H2Core::Drumkit corresponding to \a dk_name.
		 */
		static QString drumkit_path_search( const QString& dk_name, Lookup lookup = Lookup::stacked, bool bSilent = false );
		/**
		 * returns the directory holding the named drumkit searching within user then system drumkits
		 * \param dk_name the drumkit name
		 * \param lookup Where to search (system/user folder or both)
		 * for the drumkit. 
		 */
		static QString drumkit_dir_search( const QString& dk_name, Lookup lookup );
		/**
		 * returns true if the path contains a usable drumkit
		 * \param dk_path the root drumkit location
		 */
		static bool drumkit_valid( const QString& dk_path );
		/**
		 * returns the path to the xml file within a supposed drumkit path
		 * \param dk_path the path to the drumkit
		 */
		static QString drumkit_file( const QString& dk_path );

		/**
		 * Returns filename and extension of the expected drumkit file.
		 */
		static QString drumkit_xml();

		/**
		 * Create a backup path from a drumkit path. It will contain
		 * the current datetime to both make individual backup names
		 * unique and to make it more easy to handle them.
		 */
		static QString drumkit_backup_path( const QString& dk_path );

		/* PATTERNS */
		/**
		 * returns a list of existing drumkit sub dir into the patterns directory
		 */
		static QStringList pattern_drumkits();
		/**
		 * returns a list of existing patterns
		 */
		static QStringList pattern_list();
		/**
		 * returns a list of existing patterns
		 * \param path the path to look for patterns in
		 */
		static QStringList pattern_list( const QString& path );

		/* SONGS */
		/** returns a list of existing songs */
		static QStringList song_list( );
		/** returns a list of existing songs, excluding the autosaved one */
		static QStringList song_list_cleared( );
		/**
		 * returns true if the song file exists
		 * \param sg_name the song name
		 */
		static bool song_exists( const QString& sg_name );
		
		/**
		 * Checks the path pointing to a .h2song.
		 *
		 * It will be checked whether @a songPath
		 * - is absolute
		 * - exists (if @a bCheckExistance is set to true)
		 * - has the '.h2song' suffix
		 * - is writable (read-only songs are considered valid as well
		 * and the function returns true. But it also triggers an
		 * event informing the GUI to show a read-only warning.)
		 *
		 * \param sSongPath Absolute path to an .h2song file.
		 * \param bCheckExistance Whether the existence of the file is
		 * checked (should be true for opening and false for creating
		 * a new song)
		 * \return true - if valid.
		 */
		static bool isSongPathValid( const QString& sSongPath, bool bCheckExistance = false );

		/**
		 * Takes an arbitrary path, replaces white spaces by
		 * underscores and removes all characters apart from latin
		 * characters, arabic numbers, underscores and dashes.
		 */
		static QString validateFilePath( const QString& sPath );

		static QStringList theme_list();

		/** send current settings information to logger with INFO severity */
		static void info();

		/* PLAYLISTS */
		/** returns a list of existing playlists */
		static QStringList playlist_list( );

		/**
		 * returns true if the given path is an existing regular file
		 * \param path the path to the file to check
		 * \param silent output not messages if set to true
		 */
		static bool file_exists( const QString& path, bool silent=false );
		/**
		 * returns true if the given path is an existing readable regular file
		 * \param path the path to the file to check
		 * \param silent output not messages if set to true
		 */
		static bool file_readable( const QString& path, bool silent=false );
		/**
		 * returns true if the given path is a possibly writable file (may exist or not)
		 * \param path the path to the file to check
		 * \param silent output not messages if set to true
		 */
		static bool file_writable( const QString& path, bool silent=false );
		/**
		 * returns true if the given path is an existing executable regular file
		 * \param path the path to the file to check
		 * \param silent output not messages if set to true
		 */
		static bool file_executable( const QString& path, bool silent=false );
		/**
		 * returns true if the given path is a readable regular directory
		 * \param path the path to the file to check
		 * \param silent output not messages if set to true
		 */
		static bool dir_readable( const QString& path, bool silent=false );
		/**
		 * returns true if the given path is a writable regular directory
		 * \param path the path to the file to check
		 * \param silent output not messages if set to true
		 */
		static bool dir_writable( const QString& path, bool silent=false );
		/**
		 * returns true if the path is a readable and writable regular directory, create if it not exists
		 * \param path the path to the file to check
		 * \param create will try to create path if not exists and set to true
		 * \param silent output not messages if set to true
		 */
		static bool path_usable( const QString& path, bool create=true, bool silent=false );

		/**
		 * Convert a direct to an absolute path.
		 */
		static QString absolute_path( const QString& sFilename, bool bSilent = false );
		/**
		 * writes to a file
		 * \param dst the destination path
		 * \param content then string to write
		 */
		static bool write_to_file( const QString& dst, const QString& content );
		/**
		 * copy a source file to a destination
		 * \param src source file path
		 * \param dst destination file path
		 * \param overwrite allow to overwrite an existing file if set to true
		 * \param bSilent Whether debug and info messages should be
		 * logged.
		 */
		static bool file_copy( const QString& src, const QString& dst, bool overwrite=false, bool bSilent = false );
		/**
		 * remove a path
		 * \param path the path to be removed
		 * \param recursive perform recursive removal if set to true
		 * \param bSilent Whether debug and info messages should be
		 * logged.
		 */
		static bool rm( const QString& path, bool recursive=false, bool bSilent = false );
		/**
		 * create a path
		 * \param path the path to the directory to be created
		 */
		static bool mkdir( const QString& path );

		/** \return m_sPreferencesOverwritePath*/
		static const QString& getPreferencesOverwritePath();
		/** \param sPath Sets m_sPreferencesOverwritePath*/
		static void setPreferencesOverwritePath( const QString& sPath );

	private:
		static Logger* __logger;                    ///< a pointer to the logger
		static bool check_sys_paths();              ///< returns true if the system path is consistent
		static bool check_usr_paths();              ///< returns true if the user path is consistent
		static bool rm_fr( const QString& path, bool bSilent = false );   ///< recursively remove a path

		/**
		 * If this variable is non-empty, its content will be used as
		 * an alternative to store and load the preferences.*/
		static QString m_sPreferencesOverwritePath;
		/**
		 * \return a list of usable drumkits, which means having a readable drumkit.xml file
		 * \param path the path to search in for drumkits
		 */
		static QStringList drumkit_list( const QString& path );
		/**
		 * \return true if all the asked permissions are ok
		 * \param path the path to the file to check
		 * \param perms bit mask of file_perms
		 * \param silent output not messages if set to true
		 */
		static bool check_permissions( const QString& path, const int perms, bool silent );

		/**
		 * Update the data, config and cache paths with QStandardPaths if old
		 * folder doesn't exist (e.g. ~/.hydrogen/).
		 */
		static void update_usr_paths();

		/**
		 * Path to the system files set in Filesystem::bootstrap().
		 *
		 * If Q_OSMACX is set, it will be a concatenation of
		 * QCoreApplication::applicationDirPath() and
		 * "/../Resources/data/" (H2CORE_HAVE_BUNDLE defined)
		 * or "/data/" (else). If, instead, WIN32 is set, it
		 * is a concatenation of
		 * QCoreApplication::applicationDirPath() and
		 * "/data/". In case the application is neither run on
		 * Mac or Windows, it is set to a concatenation of
		 * H2_SYS_PATH and "/data/".
		 *
		 * If Filesystem::bootstrap() was called with the @a
		 * sys_path argument preset, it will overwrite all the
		 * choices above. 
		 *
		 * Finally, if the variable doesn't point to a
		 * readable directory afterwards, it is set to a
		 * concatenation of
		 * QCoreApplication::applicationDirPath(), "/", and
		 * LOCAL_DATA_PATH.
		 */
		static QString __sys_data_path;     ///< the path to the system files
		static QString __usr_data_path;     ///< the path to the user files
		static QString __usr_cache_path;    ///< the path to the cache files
		static QString __usr_cfg_path;      ///< the path to the user config file
		static QString __usr_log_path;      ///< the path to the log file
		static QStringList __ladspa_paths;  ///< paths to laspa plugins
	};

	inline const QString& Filesystem::getPreferencesOverwritePath() {
		return Filesystem::m_sPreferencesOverwritePath;
	}
	inline void Filesystem::setPreferencesOverwritePath( const QString& sPath ) {
		Filesystem::m_sPreferencesOverwritePath = sPath;
	}

};
#endif  // H2C_FILESYSTEM_H

/* vim: set softtabstop=4 noexpandtab: */
