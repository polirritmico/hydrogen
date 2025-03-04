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

#include <core/Basics/Drumkit.h>
#include <core/config.h>
#include <core/EventQueue.h>
#include <core/Helpers/Filesystem.h>
#include <core/Hydrogen.h>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QCoreApplication>
#include <QDateTime>
#if defined(Q_OS_MACX) || defined(WIN32)
#else
#include <QStandardPaths>
#endif

#ifdef H2CORE_HAVE_OSC
#include <core/NsmClient.h>
#endif

// directories
#define LOCAL_DATA_PATH "data/"
#define CACHE           "cache/"
#define DEMOS           "demo_songs/"
#define DOC             "doc/"
#define DRUMKITS        "drumkits/"
#define I18N            "i18n/"
#define IMG             "img/"
#define PATTERNS        "patterns/"
#define PLAYLISTS       "playlists/"
#define PLUGINS         "plugins/"
#define REPOSITORIES    "repositories/"
#define SCRIPTS         "scripts/"
#define SONGS           "songs/"
#define THEMES          "themes/"
#define TMP             "hydrogen/"
#define XSD             "xsd/"


// files
/** Sound of metronome beat */
#define CLICK_SAMPLE    "click.wav"
#define EMPTY_SAMPLE    "emptySample.wav"
#define DEFAULT_SONG    "DefaultSong"
#define EMPTY_SONG_BASE "emptySong"
#define USR_CONFIG		"hydrogen.conf"
#define SYS_CONFIG		"hydrogen.default.conf"
#define LOG_FILE		"hydrogen.log"
#define DRUMKIT_XML     "drumkit.xml"
#define DRUMKIT_XSD     "drumkit.xsd"
#define DRUMPAT_XSD     "drumkit_pattern.xsd"
#define DRUMKIT_DEFAULT_KIT "GMRockKit"
#define PLAYLIST_XSD     "playlist.xsd"

#define AUTOSAVE        "autosave"

#define UNTITLED_SONG		"Untitled Song"
#define UNTITLED_PLAYLIST	"untitled.h2playlist"

// filters
#define PATTERN_FILTER  "*.h2pattern"
#define PLAYLIST_FILTER "*.h2playlist"
#define SONG_FILTER     "*.h2song"
#define THEME_FILTER     "*.h2theme"

namespace H2Core
{

Logger* Filesystem::__logger = nullptr;

const QString Filesystem::scripts_ext = ".sh";
const QString Filesystem::songs_ext = ".h2song";
const QString Filesystem::themes_ext = ".h2theme";
const QString Filesystem::patterns_ext = ".h2pattern";
const QString Filesystem::playlist_ext = ".h2playlist";
const QString Filesystem::drumkit_ext = ".h2drumkit";
const QString Filesystem::scripts_filter_name = "Hydrogen Scripts (*.sh)";
const QString Filesystem::songs_filter_name = "Hydrogen Songs (*.h2song)";
const QString Filesystem::themes_filter_name = "Hydrogen Theme (*.h2theme)";
const QString Filesystem::patterns_filter_name = "Hydrogen Patterns (*.h2pattern)";
const QString Filesystem::playlists_filter_name = "Hydrogen Playlists (*.h2playlist)";

QString Filesystem::__sys_data_path;
QString Filesystem::__usr_data_path;
QString Filesystem::__usr_cache_path;
QString Filesystem::__usr_cfg_path;

#ifdef Q_OS_MACX
	QString Filesystem::__usr_log_path =QDir::homePath().append( "/Library/Application Support/Hydrogen/" LOG_FILE );
#elif WIN32
	QString Filesystem::__usr_log_path = QDir::homePath().append( "/.hydrogen/" LOG_FILE );
#else
	QString Filesystem::__usr_log_path = QDir::homePath().append( "/" H2_USR_PATH "/" );
	bool __usr_log_path_initialized = false;
#endif


QStringList Filesystem::__ladspa_paths;

QString Filesystem::m_sPreferencesOverwritePath = "";

/* TODO QCoreApplication is not instantiated */
bool Filesystem::bootstrap( Logger* logger, const QString& sys_path )
{
	if( __logger==nullptr && logger!=nullptr ) {
		__logger = logger;
	} else {
		return false;
	}

#ifdef Q_OS_MACX
#ifdef H2CORE_HAVE_BUNDLE
	// Bundle: Prepare hydrogen to use path names which are used in app bundles: http://en.wikipedia.org/wiki/Application_Bundle
	__sys_data_path = QCoreApplication::applicationDirPath().append( "/../Resources/data/" ) ;
#else
	__sys_data_path = QCoreApplication::applicationDirPath().append( "/data/" ) ;
#endif
	__usr_data_path = QDir::homePath().append( "/Library/Application Support/Hydrogen/data/" );
	__usr_cfg_path = QDir::homePath().append( "/Library/Application Support/Hydrogen/" USR_CONFIG );
#elif WIN32
	__sys_data_path = QCoreApplication::applicationDirPath().append( "/data/" ) ;
	__usr_data_path = QDir::homePath().append( "/.hydrogen/data/" ) ;
	__usr_cfg_path = QDir::homePath().append( "/.hydrogen/" USR_CONFIG ) ;
#else
	__sys_data_path = H2_SYS_PATH "/data/";
	__usr_data_path = QDir::homePath().append( "/" H2_USR_PATH "/data/" );
	__usr_cfg_path = QDir::homePath().append( "/" H2_USR_PATH "/" USR_CONFIG );
#endif
	if( sys_path!=nullptr ) __sys_data_path = sys_path;

	if( !dir_readable( __sys_data_path ) ) {
		__sys_data_path = QCoreApplication::applicationDirPath().append( "/" LOCAL_DATA_PATH );
		ERRORLOG( QString( "will use local data path : %1" ).arg( __sys_data_path ) );
	}

	char* ladspaPath = getenv( "LADSPA_PATH" );
	if ( ladspaPath ) {
		INFOLOG( "Found LADSPA_PATH environment variable" );
		QString sLadspaPath = QString::fromLocal8Bit( ladspaPath );
		int pos;
		while ( ( pos = sLadspaPath.indexOf( ":" ) ) != -1 ) {
			QString sPath = sLadspaPath.left( pos );
			__ladspa_paths << QFileInfo(sPath).canonicalFilePath();
			sLadspaPath = sLadspaPath.mid( pos + 1, sLadspaPath.length() );
		}
		__ladspa_paths << QFileInfo( sLadspaPath ).canonicalFilePath();
	} else {
#ifdef Q_OS_MACX
		__ladspa_paths << QFileInfo( QCoreApplication::applicationDirPath(), "/../Resources/plugins" ).canonicalFilePath();
		__ladspa_paths << QFileInfo( "/Library/Audio/Plug-Ins/LADSPA/" ).canonicalFilePath();
		__ladspa_paths << QFileInfo( QDir::homePath(), "/Library/Audio/Plug-Ins/LADSPA" ).canonicalFilePath();
#else
		__ladspa_paths << QFileInfo( "/usr/lib/ladspa" ).canonicalFilePath();
		__ladspa_paths << QFileInfo( "/usr/local/lib/ladspa" ).canonicalFilePath();
		__ladspa_paths << QFileInfo( "/usr/lib64/ladspa" ).canonicalFilePath();
		__ladspa_paths << QFileInfo( "/usr/local/lib64/ladspa" ).canonicalFilePath();
#endif
	}
	__ladspa_paths.sort();
	__ladspa_paths.removeDuplicates();
	if ( !__ladspa_paths.isEmpty() && __ladspa_paths.at( 0 ).isEmpty() ) {
		__ladspa_paths.removeFirst();
	}
	// we want this first
	__ladspa_paths << Filesystem::plugins_dir();
	__ladspa_paths.removeDuplicates();

	bool ret = check_sys_paths();
	ret &= check_usr_paths();
	info();
	return ret;
}

bool Filesystem::check_permissions( const QString& path, const int perms, bool silent )
{
	QFileInfo fi( path );
	if( ( perms & is_file ) && ( perms & is_writable ) && !fi.exists() ) {
		QFileInfo folder( path.left( path.lastIndexOf( "/" ) ) );
		if( !folder.isDir() ) {
			if( !silent ) {
				ERRORLOG( QString( "%1 is not a directory" ).arg( folder.fileName() ) );
			}
			return false;
		}
		if( !folder.isWritable() ) {
			if( !silent ) {
				ERRORLOG( QString( "%1 is not writable" ).arg( folder.fileName() ) );
			}
			return false;
		}
		return true;
	}
	if( ( perms & is_dir ) && !fi.isDir() ) {
		if( !silent ) {
			ERRORLOG( QString( "%1 is not a directory" ).arg( path ) );
		}
		return false;
	}
	if( ( perms & is_file ) && !fi.isFile() ) {
		if( !silent ) {
			ERRORLOG( QString( "%1 is not a file" ).arg( path ) );
		}
		return false;
	}
	if( ( perms & is_readable ) && !fi.isReadable() ) {
		if( !silent ) {
			ERRORLOG( QString( "%1 is not readable" ).arg( path ) );
		}
		return false;
	}
	if( ( perms & is_writable ) && !fi.isWritable() ) {
		if( !silent ) {
			ERRORLOG( QString( "%1 is not writable" ).arg( path ) );
		}
		return false;
	}
	if( ( perms & is_executable ) && !fi.isExecutable() ) {
		if( !silent ) {
			ERRORLOG( QString( "%1 is not executable" ).arg( path ) );
		}
		return false;
	}
	return true;
}

bool Filesystem::file_exists( const QString& path, bool silent )
{
	return check_permissions( path, is_file, silent );
}
bool Filesystem::file_readable( const QString& path, bool silent )
{
	return check_permissions( path, is_file|is_readable, silent );
}
bool Filesystem::file_writable( const QString& path, bool silent )
{
	return check_permissions( path, is_file|is_readable|is_writable, silent );
}
bool Filesystem::file_executable( const QString& path, bool silent )
{
	return check_permissions( path, is_file|is_executable, silent );
}
bool Filesystem::dir_readable(  const QString& path, bool silent )
{
	return check_permissions( path, is_dir|is_readable|is_executable, silent );
}
bool Filesystem::dir_writable(  const QString& path, bool silent )
{
	return check_permissions( path, is_dir|is_writable, silent );
}

bool Filesystem::mkdir( const QString& path )
{
	if ( !QDir( "/" ).mkpath( QDir( path ).absolutePath() ) ) {
		ERRORLOG( QString( "unable to create directory : %1" ).arg( path ) );
		return false;
	}
	return true;
}

bool Filesystem::path_usable( const QString& path, bool create, bool silent )
{
	if ( !QDir( path ).exists() ) {
		if ( !silent ) {
			INFOLOG( QString( "create user directory : %1" ).arg( path ) );
		}
		if ( create && !QDir( "/" ).mkpath( path ) ) {
			if( !silent ) {
				ERRORLOG( QString( "unable to create user directory : %1" ).arg( path ) );
			}
			return false;
		}
	}
	return dir_readable( path, silent ) && dir_writable( path, silent );
}

bool Filesystem::write_to_file( const QString& dst, const QString& content )
{
	if ( !file_writable( dst ) ) {
		ERRORLOG( QString( "unable to write to %1" ).arg( dst ) );
		return false;
	}
	QFile file( dst );
	if ( !file.open( QIODevice::WriteOnly ) ) {
		ERRORLOG( QString( "unable to write to %1" ).arg( dst ) );
		return false;
	}
	file.write( content.toUtf8().data() );
	file.close();

	return true;
}

bool Filesystem::file_copy( const QString& src, const QString& dst, bool overwrite, bool bSilent )
{
	if( !overwrite && file_exists( dst, true ) ) {
		WARNINGLOG( QString( "do not overwrite %1 with %2 as it already exists" ).arg( dst ).arg( src ) );
		return true;
	}
	if ( !file_readable( src ) ) {
		ERRORLOG( QString( "unable to copy %1 to %2, %1 is not readable" ).arg( src ).arg( dst ) );
		return false;
	}
	if ( !file_writable( dst ) ) {
		ERRORLOG( QString( "unable to copy %1 to %2, %2 is not writable" ).arg( src ).arg( dst ) );
		return false;
	}
	if ( ! bSilent ) {
		INFOLOG( QString( "copy %1 to %2" ).arg( src ).arg( dst ) );
	}
	
	// Since QFile::copy does not overwrite, we have to make sure the
	// destination does not exist.
	if ( overwrite && file_exists( dst, true ) ) {
		rm( dst, true, bSilent );
	}
	
	return QFile::copy( src, dst );
}

bool Filesystem::rm( const QString& path, bool recursive, bool bSilent )
{
	if ( check_permissions( path, is_file, true ) ) {
		QFile file( path );
		bool ret = file.remove();
		if( !ret ) {
			ERRORLOG( QString( "unable to remove file %1" ).arg( path ) );
		}
		return ret;
	}
	if ( !check_permissions( path, is_dir, true )  ) {
		ERRORLOG( QString( "%1 is neither a file nor a directory ?!?!" ).arg( path ) );
		return false;
	}
	if ( !recursive ) {
		QDir dir;
		bool ret = dir.rmdir( path );
		if( !ret ) {
			ERRORLOG( QString( "unable to remove dir %1 without recursive argument, maybe it is not empty?" ).arg( path ) );
		}
		return ret;
	}
	return rm_fr( path, bSilent );
}

bool Filesystem::rm_fr( const QString& path, bool bSilent )
{
	if ( ! bSilent ) {
		INFOLOG( QString( "Removing [%1] recursively" ).arg( path ) );
	}
	
	bool ret = true;
	QDir dir( path );
	QFileInfoList entries = dir.entryInfoList( QDir::NoDotAndDotDot | QDir::AllEntries );
	for ( int idx = 0; ( ( idx < entries.size() ) && ret ); idx++ ) {
		QFileInfo entryInfo = entries[idx];
		if ( entryInfo.isDir() && !entryInfo.isSymLink() ) {
			ret = rm_fr( entryInfo.absoluteFilePath(), bSilent );
		} else {
			QFile file( entryInfo.absoluteFilePath() );
			if ( !file.remove() ) {
				ERRORLOG( QString( "unable to remove %1" ).arg( entryInfo.absoluteFilePath() ) );
				ret = false;
			}
		}
	}
	if ( !dir.rmdir( dir.absolutePath() ) ) {
		ERRORLOG( QString( "unable to remove %1" ).arg( dir.absolutePath() ) );
		ret = false;
	}
	return ret;
}

bool Filesystem::check_sys_paths()
{
	bool ret = true;
	if(  !dir_readable( __sys_data_path ) ) ret = false;
	if( !file_readable( click_file_path() ) ) ret = false;
	if(  !dir_readable( demos_dir() ) ) ret = false;
	/* if(  !dir_readable( doc_dir() ) ) ret = false; */		// FIXME
	if(  !dir_readable( sys_drumkits_dir() ) ) ret = false;
	if( !file_readable( empty_sample_path() ) ) ret = false;
	if( !file_readable( sys_config_path() ) ) ret = false;
	if(  !dir_readable( i18n_dir() ) ) ret = false;
	if(  !dir_readable( img_dir() ) ) ret = false;
	if(  !dir_readable( sys_theme_dir() ) ) ret = false;
	if(  !dir_readable( xsd_dir() ) ) ret = false;
	if( !file_readable( pattern_xsd_path() ) ) ret = false;
	if( !file_readable( drumkit_xsd_path() ) ) ret = false;
	if( !file_readable( playlist_xsd_path() ) ) ret = false;

	if ( ret ) {
		INFOLOG( QString( "system wide data path %1 is usable." ).arg( __sys_data_path ) );
	}
	
	return ret;
}


#if defined(Q_OS_MACX) || defined(WIN32)
#else
void Filesystem::update_usr_paths()
{
	// If the old path exists (e.g. ~/.hydrogen), old path is used; else uses
	// QStandardPaths to get XDG Paths on Linux.
	if ( !QFileInfo::exists( QFileInfo(__usr_cfg_path).absolutePath() ) )
	{
		__usr_cfg_path = QStandardPaths::writableLocation(
				QStandardPaths::AppConfigLocation).append("/" USR_CONFIG);
		__usr_data_path = QStandardPaths::writableLocation(
				QStandardPaths::AppLocalDataLocation).append("/");
		__usr_cache_path = QStandardPaths::writableLocation(
				QStandardPaths::CacheLocation).append("/");
	} else {
		__usr_cache_path = __usr_data_path + CACHE;
	}
}
#endif


bool Filesystem::check_usr_paths()
{
	bool ret = true;

#if defined(Q_OS_MACX) || defined(WIN32)
#else
	update_usr_paths();
	if ( !path_usable( QFileInfo(__usr_cfg_path).absolutePath() ) ) ret = false;
#endif
	if( !path_usable( tmp_dir() ) ) ret = false;
	if( !path_usable( __usr_data_path ) ) ret = false;
	if( !path_usable( cache_dir() ) ) ret = false;
	if( !path_usable( repositories_cache_dir() ) ) ret = false;
	if( !path_usable( usr_drumkits_dir() ) ) ret = false;
	if( !path_usable( patterns_dir() ) ) ret = false;
	if( !path_usable( playlists_dir() ) ) ret = false;
	if( !path_usable( plugins_dir() ) ) ret = false;
	if( !path_usable( scripts_dir() ) ) ret = false;
	if( !path_usable( songs_dir() ) ) ret = false;
	if( file_exists( empty_song_path(), true ) ) ret = false;
	if( !path_usable( usr_theme_dir() ) ) ret = false;
	if( !file_writable( usr_config_path() ) ) ret = false;

	if ( ret ) {
		INFOLOG( QString( "user path %1 is usable." ).arg( __usr_data_path ) );
	}
	
	return ret;
}

QString Filesystem::sys_data_path()
{
	return __sys_data_path;
}
QString Filesystem::usr_data_path()
{
	return __usr_data_path;
}

QStringList Filesystem::ladspa_paths()
{
	return __ladspa_paths;
}

// FILES
QString Filesystem::sys_config_path()
{
       return __sys_data_path + SYS_CONFIG;
}
QString Filesystem::usr_config_path()
{
       return __usr_cfg_path;
}
QString Filesystem::empty_sample_path()
{
	return __sys_data_path + EMPTY_SAMPLE;
}

QString Filesystem::default_song_name() {
	return DEFAULT_SONG;
}

QString Filesystem::empty_song_path() {
	QString sPathBase( __usr_data_path + EMPTY_SONG_BASE );
	QString sPath( sPathBase + Filesystem::songs_ext );

	int nIterations = 0;
	while ( file_exists( sPath, true ) ) {
		sPath = sPathBase + QString::number( nIterations ) + Filesystem::songs_ext;
		++nIterations;

		if ( nIterations > 1000 ) {
			ERRORLOG( "That's a bit much. Something is wrong in here." );
			return __usr_data_path + SONGS + default_song_name() +
				Filesystem::songs_ext;
		}
	}

	return sPath;
}

QString Filesystem::untitled_song_name()
{
	return UNTITLED_SONG;
}
QString Filesystem::untitled_playlist_file_name()
{
	return UNTITLED_PLAYLIST;
}
QString Filesystem::click_file_path()
{
	return __sys_data_path + CLICK_SAMPLE;
}
QString Filesystem::usr_click_file_path()
{
	if( file_readable( __usr_data_path + CLICK_SAMPLE, true ) ) return __usr_data_path + CLICK_SAMPLE;
	return click_file_path();
}
QString Filesystem::drumkit_xsd_path( )
{
	return xsd_dir() + DRUMKIT_XSD;
}
QString Filesystem::pattern_xsd_path( )
{
	return xsd_dir() + DRUMPAT_XSD;
}
QString Filesystem::playlist_xsd_path( )
{
	return xsd_dir() + PLAYLIST_XSD;
}
QString Filesystem::log_file_path()
{
#if defined(Q_OS_MACX) || defined(WIN32)
#else
	if ( !__usr_log_path_initialized )
	{
		if ( !QFileInfo::exists( __usr_log_path ) )
		{
			__usr_log_path = QStandardPaths::writableLocation(
					QStandardPaths::AppLocalDataLocation).append( "/" LOG_FILE );
		} else {
			__usr_log_path.append( LOG_FILE );
		}
		__usr_log_path_initialized = true;
	}
#endif
	return __usr_log_path;
}

// DIRS
QString Filesystem::img_dir()
{
	return __sys_data_path + IMG;
}
QString Filesystem::doc_dir()
{
	return __sys_data_path + DOC;
}
QString Filesystem::i18n_dir()
{
	return __sys_data_path + I18N;
}
QString Filesystem::scripts_dir()
{
	return __usr_data_path + SCRIPTS;
}
QString Filesystem::songs_dir()
{
	return __usr_data_path + SONGS;
}
QString Filesystem::usr_theme_dir()
{
	return __usr_data_path + THEMES;
}
QString Filesystem::sys_theme_dir()
{
	return __sys_data_path + THEMES;
}
QString Filesystem::song_path( const QString& sg_name )
{
	return QString( songs_dir() + sg_name + songs_ext );
}
QString Filesystem::patterns_dir()
{
	return __usr_data_path + PATTERNS;
}
QString Filesystem::patterns_dir( const QString& dk_name )
{
	return __usr_data_path + PATTERNS + dk_name + "/";
}
QString Filesystem::pattern_path( const QString& dk_name, const QString& p_name )
{
	if ( dk_name.isEmpty() ) {
		return patterns_dir() + p_name + patterns_ext;
	} else {
		return patterns_dir( dk_name ) + p_name + patterns_ext;
	}
}
QString Filesystem::plugins_dir()
{
	return __usr_data_path + PLUGINS;
}
QString Filesystem::sys_drumkits_dir()
{
	return __sys_data_path + DRUMKITS;
}
QString Filesystem::usr_drumkits_dir()
{
	return __usr_data_path + DRUMKITS;
}
QString Filesystem::playlists_dir()
{
	return __usr_data_path + PLAYLISTS;
}
QString Filesystem::playlist_path( const QString& pl_name )
{
	return patterns_dir() + pl_name + playlist_ext;
}
QString Filesystem::cache_dir()
{
#if defined(Q_OS_MACX) || defined(WIN32)
	return __usr_data_path + CACHE;
#else
	return __usr_cache_path;
#endif
}
QString Filesystem::repositories_cache_dir()
{
#if defined(Q_OS_MACX) || defined(WIN32)
	return __usr_data_path + CACHE + REPOSITORIES;
#else
	return __usr_cache_path + REPOSITORIES;
#endif
}
QString Filesystem::demos_dir()
{
	return __sys_data_path + DEMOS;
}
QString Filesystem::xsd_dir()
{
	return __sys_data_path + XSD;
}
QString Filesystem::tmp_dir()
{
	return QDir::tempPath() + "/" + TMP;
}
QString Filesystem::tmp_file_path( const QString &base )
{
	// Ensure template base will produce a valid filename
	QString validBase = base;
	validBase.remove( QRegExp( "[^a-zA-Z0-9._]" ) );

	QFileInfo f( validBase );
	QString templateName( tmp_dir() + "/" );
	if ( f.suffix().isEmpty() ) {
		templateName += validBase.left( 20 );
	} else {
		templateName += f.completeBaseName().left( 20 ) + "-XXXXXX." + f.suffix();
	}
	QTemporaryFile file( templateName);
	file.setAutoRemove( false );
	file.open();
	file.close();
	return file.fileName();
}

// DRUMKITS
QStringList Filesystem::drumkit_list( const QString& path )
{
	QStringList ok;
	QStringList possible = QDir( path ).entryList( QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot );
	foreach ( const QString& dk, possible ) {
		if ( drumkit_valid( path + dk ) ) {
			ok << dk;
		} else {
			ERRORLOG( QString( "drumkit %1 is not usable" ).arg( dk ) );
		}
	}
	return ok;
}
QString Filesystem::drumkit_default_kit() {
	return DRUMKIT_DEFAULT_KIT;
}
QStringList Filesystem::sys_drumkit_list( )
{
	return drumkit_list( sys_drumkits_dir() ) ;
}
QStringList Filesystem::usr_drumkit_list( )
{
	return drumkit_list( usr_drumkits_dir() ) ;
}

QString Filesystem::prepare_sample_path( const QString& fname )
{
	int idx = get_basename_idx_under_drumkit( fname );
	if ( idx >= 0 ) {
		return fname.midRef( idx ).toString();
	}
	return fname;
}

bool Filesystem::file_is_under_drumkit( const QString& fname )
{
	return get_basename_idx_under_drumkit( fname ) != -1;
}

int Filesystem::get_basename_idx_under_drumkit( const QString& fname )
{
	if( fname.startsWith( usr_drumkits_dir() ) )
	{
		int start = usr_drumkits_dir().size();
		int index = fname.indexOf( "/", start );
		QString dk_name = fname.midRef( start , index - start).toString();
		if ( usr_drumkit_list().contains( dk_name ) ) {
			return index + 1;
		}
	}

	if( fname.startsWith( sys_drumkits_dir() ) )
	{
		int start = sys_drumkits_dir().size();
		int index = fname.indexOf( "/", start);
		QString dk_name = fname.midRef( start, index - start).toString();
		if ( sys_drumkit_list().contains( dk_name ) ) {
			return index + 1;
		}
	}

	return -1;
}


bool Filesystem::drumkit_exists( const QString& dk_name )
{
	if( usr_drumkit_list().contains( dk_name ) ) return true;
	return sys_drumkit_list().contains( dk_name );
}
QString Filesystem::drumkit_usr_path( const QString& dk_name )
{
	return usr_drumkits_dir() + dk_name;
}
QString Filesystem::drumkit_path_search( const QString& dk_name, Lookup lookup, bool bSilent )
{

#ifdef H2CORE_HAVE_OSC
	// When under session management the drumkit can also be located
	// in - apart from the user and system path - a particular session
	// folder. If it couldn't be found in there (or the found drumkit
	// does not match `dk_name`), the session folder is skipped and
	// the user and system paths will be traversed instead.
	if ( Hydrogen::get_instance()->isUnderSessionManagement() ) {
		
		QString sDrumkitPath = QString( "%1/%2" )
			.arg( NsmClient::get_instance()->m_sSessionFolderPath )
			.arg( "drumkit" );
		
		// If the path is symbolic link, dereference it.
		QFileInfo drumkitPathInfo( sDrumkitPath );
		if ( drumkitPathInfo.isSymLink() ) {
			sDrumkitPath = drumkitPathInfo.symLinkTarget();
		}
		
		// Check whether the local drumkit does hold the right
		// drumkit (using its name).
		QString sDrumkitXMLPath = QString( "%1/%2" )
				.arg( sDrumkitPath ).arg( "drumkit.xml" );

		if ( dk_name == Drumkit::loadNameFrom( sDrumkitXMLPath ) ) {
				// The local drumkit seems legit.	
				return sDrumkitPath;
		}
		else if ( ! bSilent ) {
			NsmClient::printError( QString( "Local drumkit [%1] and the one referenced in the .h2song file [%2] do not match!" )
								   .arg( sDrumkitXMLPath )
								   .arg( dk_name ) );
		}
	}
			
#endif
	
	if ( lookup == Lookup::stacked || lookup == Lookup::user ) {
		if ( usr_drumkit_list().contains( dk_name ) ){
			return usr_drumkits_dir() + dk_name;
		}
	}

	if ( lookup == Lookup::stacked || lookup == Lookup::system ) {
		if( sys_drumkit_list().contains( dk_name ) ){
			return sys_drumkits_dir() + dk_name;
		}
	}

	if ( ! bSilent ) {
		ERRORLOG( QString( "drumkit %1 not found using lookup type [%2]" )
				  .arg( dk_name )
				  .arg( static_cast<int>(lookup)));
	}
	
	return QString("");
}
	
QString Filesystem::drumkit_dir_search( const QString& dk_name, Lookup lookup )
{
	if ( lookup == Lookup::user || lookup == Lookup::stacked ) {
		if ( usr_drumkit_list().contains( dk_name ) ) {
			return usr_drumkits_dir();
		}
	}
	if ( lookup == Lookup::system || lookup == Lookup::stacked ) {
		if( sys_drumkit_list().contains( dk_name ) ) {
			return sys_drumkits_dir();
		}
	}
	ERRORLOG( QString( "drumkit %1 not found with lookup mode [%2]" )
			  .arg( dk_name ).arg( static_cast<int>(lookup) ) );
	return "";
}
bool Filesystem::drumkit_valid( const QString& dk_path )
{
	return file_readable( dk_path + "/" + DRUMKIT_XML, true);
}
QString Filesystem::drumkit_file( const QString& dk_path )
{
	return dk_path + "/" + DRUMKIT_XML;
}

QString Filesystem::drumkit_xml() {
	return DRUMKIT_XML;
}

QString Filesystem::drumkit_backup_path( const QString& dk_path ) {
	return dk_path + "." +
		QDateTime::currentDateTime().toString( "yyyy-MM-dd_hh-mm-ss" ) + ".bak";
}

// PATTERNS
QStringList Filesystem::pattern_drumkits()
{
	return QDir( patterns_dir() ).entryList( QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot );
}

QStringList Filesystem::pattern_list()
{
	return pattern_list( patterns_dir() );
}

QStringList Filesystem::pattern_list( const QString& path)
{
	return QDir( path ).entryList( QStringList( PATTERN_FILTER ), QDir::Files | QDir::Readable | QDir::NoDotAndDotDot );
}

// SONGS
QStringList Filesystem::song_list( )
{
	return QDir( songs_dir() ).entryList( QStringList( SONG_FILTER ), QDir::Files | QDir::Readable | QDir::NoDotAndDotDot );
}

QStringList Filesystem::song_list_cleared( )
{
	QStringList result;
	foreach ( const QString& str, song_list() ) {
		if ( !str.contains( AUTOSAVE ) ) {
			result += str;
		}
	}
	return result;
}

bool Filesystem::song_exists( const QString& sg_name )
{
	return QDir( songs_dir() ).exists( sg_name );
}

bool Filesystem::isSongPathValid( const QString& sSongPath, bool bCheckExistance ) {
	
	QFileInfo songFileInfo = QFileInfo( sSongPath );

	if ( !songFileInfo.isAbsolute() ) {
		ERRORLOG( QString( "Error: Unable to handle path [%1]. Please provide an absolute file path!" )
						.arg( sSongPath.toLocal8Bit().data() ));
		return false;
	}
	
	if ( songFileInfo.exists() ) {
		if ( !songFileInfo.isReadable() ) {
			ERRORLOG( QString( "Unable to handle path [%1]. You must have permissions to read the file!" )
						.arg( sSongPath.toLocal8Bit().data() ));
			return false;
		}
		if ( !songFileInfo.isWritable() ) {
			WARNINGLOG( QString( "You don't have permissions to write to the Song found in path [%1]. It will be opened as read-only (no autosave)." )
						.arg( sSongPath.toLocal8Bit().data() ));
			EventQueue::get_instance()->push_event( EVENT_UPDATE_SONG, 2 );
		}
	} else if ( bCheckExistance ) {
		ERRORLOG( QString( "Provided song [%1] does not exist" ).arg( sSongPath ) );
		return false;
	}
	
	if ( songFileInfo.suffix() != "h2song" ) {
		ERRORLOG( QString( "Unable to handle path [%1]. The provided file must have the suffix '.h2song'!" )
					.arg( sSongPath.toLocal8Bit().data() ));
		return false;
	}
	
	return true;
}

QString Filesystem::validateFilePath( const QString& sPath ) {

	// Ensure the name will be a valid filename
	QString sValidName( sPath );
	sValidName.replace( " ", "_" );
	sValidName.remove( QRegExp( "[^a-zA-Z0-9_-]" ) );

	return sValidName;
}

QStringList Filesystem::theme_list( )
{
	return QDir( sys_theme_dir() ).entryList( QStringList( THEME_FILTER ), QDir::Files | QDir::Readable | QDir::NoDotAndDotDot ) +
		QDir( usr_theme_dir() ).entryList( QStringList( THEME_FILTER ), QDir::Files | QDir::Readable | QDir::NoDotAndDotDot );
}

// PLAYLISTS
QStringList Filesystem::playlist_list( )
{
	return QDir( playlists_dir() ).entryList( QStringList( PLAYLIST_FILTER ), QDir::Files | QDir::Readable | QDir::NoDotAndDotDot );
}

void Filesystem::info()
{
	INFOLOG( QString( "Tmp dir                    : %1" ).arg( tmp_dir() ) );
	// SYS
	INFOLOG( QString( "Click file                 : %1" ).arg( click_file_path() ) );
	INFOLOG( QString( "Empty song                 : %1" ).arg( empty_song_path() ) );
	INFOLOG( QString( "Demos dir                  : %1" ).arg( demos_dir() ) );
	INFOLOG( QString( "Documentation dir          : %1" ).arg( doc_dir() ) );					// FIXME must be created even if no doc deployed
	INFOLOG( QString( "System drumkit dir         : %1" ).arg( sys_drumkits_dir() ) );
	INFOLOG( QString( "Empty sample               : %1" ).arg( empty_sample_path() ) );
	INFOLOG( QString( "Default config             : %1" ).arg( sys_config_path() ) );
	INFOLOG( QString( "Internationalization dir   : %1" ).arg( i18n_dir() ) );
	INFOLOG( QString( "Images dir                 : %1" ).arg( img_dir() ) );
	// new_tutorial
	INFOLOG( QString( "XSD dir                    : %1" ).arg( xsd_dir() ) );
	INFOLOG( QString( "drumkit pattern XSD        : %1" ).arg( pattern_xsd_path() ) );
	INFOLOG( QString( "drumkit XSD                : %1" ).arg( drumkit_xsd_path() ) );
	INFOLOG( QString( "drumkit XSD                : %1" ).arg( playlist_xsd_path() ) );
	// USR
	INFOLOG( QString( "User config                : %1" ).arg( usr_config_path() ) );			// FIXME
	INFOLOG( QString( "User Click file            : %1" ).arg( usr_click_file_path() ) );
	INFOLOG( QString( "Cache dir                  : %1" ).arg( cache_dir() ) );
	INFOLOG( QString( "Reporitories Cache dir     : %1" ).arg( repositories_cache_dir() ) );
	INFOLOG( QString( "User drumkit dir           : %1" ).arg( usr_drumkits_dir() ) );
	INFOLOG( QString( "Patterns dir               : %1" ).arg( patterns_dir() ) );
	INFOLOG( QString( "Playlist dir               : %1" ).arg( playlists_dir() ) );
	INFOLOG( QString( "Plugins dir                : %1" ).arg( plugins_dir() ) );
	INFOLOG( QString( "Scripts dir                : %1" ).arg( scripts_dir() ) );
	INFOLOG( QString( "Songs dir                  : %1" ).arg( songs_dir() ) );
}

QString Filesystem::absolute_path( const QString& sFilename, bool bSilent ) {
	if ( QFile( sFilename ).exists() ) {
		return QFileInfo( sFilename ).absoluteFilePath();
	}
	else if ( ! bSilent ) {
		___ERRORLOG( QString( "File [%1] not found" ).arg( sFilename ) );
	}

	return QString();
}
};

/* vim: set softtabstop=4 noexpandtab: */
