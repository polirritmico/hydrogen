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

#ifndef H2C_DRUMKIT_H
#define H2C_DRUMKIT_H

#include <core/Object.h>
#include <core/Helpers/Filesystem.h>
#include <core/License.h>
#include <core/Basics/InstrumentList.h>

namespace H2Core
{

class XMLDoc;
class XMLNode;
class DrumkitComponent;

/**
 * Drumkit info
*/
/** \ingroup docCore docDataStructure */
class Drumkit : public H2Core::Object<Drumkit>
{
		H2_OBJECT(Drumkit)
	public:
		/** drumkit constructor, does nothing */
		Drumkit();
		/** copy constructor */
		Drumkit( Drumkit* other );
		/** drumkit destructor, delete #__instruments */
		~Drumkit();

		/**
		 * Load drumkit information from a directory.
		 *
		 * This function is a wrapper around load_file(). The
		 * provided drumkit directory @a dk_dir is converted
		 * by Filesystem::drumkit_file() internally.
		 *
		 * \param dk_dir A directory containing a drumkit,
		 * like those returned by
		 * Filesystem::drumkit_dir_search().
		 * \param load_samples Automatically load sample data
		 * if set to true.
		 * \param bUpgrade Whether the loaded drumkit should be
		 * upgraded using upgrade_drumkit() in case it did not comply
		 * with the current XSD file.
		 * \param bSilent if set to true, all log messages except of
		 * errors and warnings are suppressed.
		 * \param lookup Where to search (system/user folder or both)
		 * for the drumkit.
		 *
		 * \return A Drumkit on success, nullptr otherwise.
		 */
		static Drumkit* load( const QString& dk_dir,
							  const bool load_samples = false,
							  bool bUpgrade = true,
							  bool bSilent = false,
							  Filesystem::Lookup lookup = Filesystem::Lookup::stacked );
		/**
		 * Simple wrapper for load() used with the drumkit's
		 * name instead of its directory.
		 *
		 * Uses Filesystem::drumkit_path_search() to determine
		 * the directory of the Drumkit from @a dk_name.
		 *
		 * \param dk_name Name of the Drumkit.
		 * \param load_samples Automatically load sample data
		 * if set to true.
		 * \param lookup Where to search (system/user folder or both)
		 * for the drumkit.
		 *
		 * \return A Drumkit on success, nullptr otherwise.
		 */
		static Drumkit* load_by_name( const QString& dk_name,
									  const bool load_samples = false,
									  Filesystem::Lookup lookup = Filesystem::Lookup::stacked );
		/**
		 * Load a Drumkit from a file.
		 *
		 * \param dk_path is a path to an xml file
		 * \param load_samples automatically load sample data if set
		 * to true
		 * \param bUpgrade Whether the loaded drumkit should be
		 * upgraded using upgrade_drumkit() in case it did not comply
		 * with the current XSD file.
		 * \param bSilent if set to true, all log messages except of
		 * errors and warnings are suppressed.
		 * \param lookup Where to search (system/user folder or both)
		 * for the drumkit.
		 *
		 * \return A Drumkit on success, nullptr otherwise.
		 */
		static Drumkit* load_file( const QString& dk_path,
								   const bool load_samples = false,
								   bool bUpgrade = true,
								   bool bSilent = true,
								   Filesystem::Lookup lookup = Filesystem::Lookup::stacked );
		/** Calls the InstrumentList::load_samples() member
		 * function of #__instruments.
		 */
		void load_samples();
		/** Calls the InstrumentList::unload_samples() member
		 * function of #__instruments.
		 */
		void unload_samples();

	/**
	 * Loads the license information of a drumkit contained in
	 * directory @a sDrumkitDir.
	 *
	 * This function is a wrapper around load_file(). The
	 * provided drumkit directory @a dk_dir is converted
	 * by Filesystem::drumkit_file() internally.
	 *
	 * \param sDrumkitDir Directory containing a drumkit.xml file.
	 */
	static License loadLicenseFrom( const QString& sDrumkitDir, bool bSilent = false );
	/**
	 * Simple wrapper for loadLicense() used with the drumkit's
	 * name instead of its directory.
	 *
	 * Uses Filesystem::drumkit_path_search() to determine
	 * the directory of the Drumkit from @a sDrumkitName.
	 *
	 * \param sDrumkitName Name of the Drumkit.
	 * \param lookup Where to search (system/user folder or both)
	 * for the drumkit.
	 */
	static License loadLicenseByNameFrom( const QString& sDrumkitName,
										  Filesystem::Lookup lookup = Filesystem::Lookup::stacked,
										  bool bSilent = false );

	/**
	 * Retrieve the name of a drumkit stored in @a sDrumkitDir.
	 *
	 * As the name of the drumkit can be set to arbitrary values, it
	 * can not be assumed to be unique and does not qualify as unique
	 * identifier of the kit. Instead, the location the drumkit is
	 * loaded from/written to is used and this function maps it to the
	 * corresponding drumkit name.
	 */
	static QString loadNameFrom( const QString& sDrumkitDir,
								 bool bSilent = false );
	
	/**
	 * Returns a version of #__name stripped of all whitespaces and
	 * other characters which would prevent its use as a valid
	 * filename.
	 *
	 * Attention: The returned string might be used as the name for
	 * the associated drumkit folder but it does not have to.
	 */
	QString getFolderName() const;
	/**
	 * Returns the base name used when exporting the drumkit.
	 *
	 * \param sComponentName Name of a particular component used in
	 * case just a single component should be exported.
	 * \param bRecentVersion Whether the drumkit format should be
	 * supported by Hydrogen 0.9.7 or higher (whether it should be
	 * composed of DrumkitComponents).
	 */
	QString getExportName( const QString& sComponentName, bool bRecentVersion ) const;
		
		/** 
		 * Saves the current drumkit to dk_path, but makes a backup. 
		 * This is used when the drumkit did not comply to 
		 * our xml schema.
		 */
		static void upgrade_drumkit( Drumkit* pDrumkit, const QString& dk_path, bool bSilent = false );

		/**
		 * check if a user drumkit with the given name
		 * already exists
		 * \param dk_path Drumkit path
		 * \return true on success
		 */
		static bool user_drumkit_exists( const QString& dk_path );

		/**
		 * save a drumkit, xml file and samples
		 * \param overwrite allows to write over existing drumkit files
		 * \return true on success
		 */
		bool save( bool overwrite=false );
		/**
		 * save a drumkit, xml file and samples
		 * neither #__path nor #__name are updated
		 * \param dk_dir the directory to save the drumkit into
		 * \param overwrite allows to write over existing drumkit files
		 * \return true on success
		 */
		bool save( const QString& dk_dir, bool overwrite=false );
		/**
		 * save a drumkit into an xml file
		 * \param dk_path the path to save the drumkit into
		 * \param overwrite allows to write over existing drumkit file
		 * \param component_id to chose the component to save or -1 for all
		 * \param bSilent if set to true, all log messages except of
		 * errors and warnings are suppressed.
		 *
		 * \return true on success
		 */
		bool save_file( const QString& dk_path, bool overwrite=false,
						int component_id=-1, bool bRecentVersion = true,
						bool bSilent = false ) const;
		/**
		 * save a drumkit instruments samples into a directory
		 * \param dk_dir the directory to save the samples into
		 * \param overwrite allows to write over existing drumkit samples files
		 * \return true on success
		 */
		bool save_samples( const QString& dk_dir, bool overwrite=false );
		/**
		 * save the drumkit image into the new directory
		 * \param dk_dir the directory to save the image into
		 * \param overwrite allows to write over existing drumkit image file
		 * \return true on success
		 */
		bool save_image( const QString& dk_dir, bool overwrite=false );
		/**
		 * save a drumkit using given parameters and an instrument list
		 * \param sName the name of the drumkit
		 * \param sAuthor the author of the drumkit
		 * \param sInfo the info of the drumkit
		 * \param license the license of the drumkit
		 * \param sImage the image filename (with full path) of
		   the drumkit
		 * \param imageLicense license of the supplied image
		 * \param pInstruments the instruments to be saved
		   within the drumkit
		 * \param pComponents
		 * \param bOverwrite allows to write over existing drumkit files
		 * \return true on success
		 */
		static bool save( const QString& sName, const QString& sAuthor, const QString& sInfo, const License& license, const QString& sImage, const License& imageLicense, InstrumentList* pInstruments, std::vector<DrumkitComponent*>* pComponents, bool bOverwrite=false );
		/**
		 * Extract a .h2drumkit file.
		 *
		 * \param sSourcePath Absolute path to the new drumkit archive
		 * \param sTargetPath Absolute path to where the new drumkit
		 * should be extracted to. If left empty, the user's drumkit
		 * folder will be used.
		 * \param bSilent Whether debug and info messages should be
		 * logged.
		 *
		 * \return true on success
		 */
	static bool install( const QString& sSourcePath, const QString& sTargetPath = "", bool bSilent = false );

	/**
	 * Compresses the drumkit into a .h2drumkit file.
	 *
	 * The name of the created file will be a concatenation of #__name
	 * and Filesystem::drumkit_ext.
	 *
	 * exportTo() ? well, export is a protected name within C++. So,
	 * we needed a less obvious name. 
	 *
	 * \param sTargetDir Folder which will contain the resulting
	 * .h2drumkit file.
	 * \param sComponentName Name of a particular component used in
	 * case just a single component should be exported.
	 * \param bRecentVersion Whether the drumkit format should be
	 * supported by Hydrogen 0.9.7 or higher (whether it should be
	 * composed of DrumkitComponents).
	 * \param bSilent Whether debug and info messages should be
	 * logged.
	 *
	 * \return true on success 
	 */
	bool exportTo( const QString& sTargetDir, const QString& sComponentName = "", bool bRecentVersion = true, bool bSilent = false );
		/**
		 * remove a drumkit from the disk
		 * \param dk_name the drumkit name
		 * \param lookup Where to search (system/user folder or both)
		 * for the drumkit.
		 * \return true on success
		 */
		static bool remove( const QString& dk_name, Filesystem::Lookup lookup );

		/** set __instruments, delete existing one */
		void set_instruments( InstrumentList* instruments );
		/**  returns #__instruments */
		InstrumentList* get_instruments() const;

		/** #__path setter */
		void set_path( const QString& path );
		/** #__path accessor */
		const QString& get_path() const;
		/** #__name setter */
		void set_name( const QString& name );
		/** #__name accessor */
		const QString& get_name() const;
		/** #__author setter */
		void set_author( const QString& author );
		/** #__author accessor */
		const QString& get_author() const;
		/** #__info setter */
		void set_info( const QString& info );
		/** #__info accessor */
		const QString& get_info() const;
		/** #__license setter */
		void set_license( const License& license );
		/** #__license accessor */
		const License& get_license() const;
		/** #__image setter */
		void set_image( const QString& image );
		/** #__image accessor */
		const QString& get_image() const;
		/** #__imageLicense setter */
		void set_image_license( const License& imageLicense );
		/** #__imageLicense accessor */
		const License& get_image_license() const;
		/** return true if the samples are loaded */
		const bool samples_loaded() const;

		void dump();

		/**
		 * \return Whether the associated files are located in the
		 * user or the systems drumkit folder.
		 */
		bool isUserDrumkit() const;

		std::vector<DrumkitComponent*>* get_components();
		void set_components( std::vector<DrumkitComponent*>* components );

	/**
	 * Assign the license stored in #m_license to all samples
	 * contained in the kit.
	 */
	void propagateLicense();
	/**
	 * Returns vector of lists containing instrument name, component
	 * name, file name, the license of all associated samples.
	 */
	std::vector<std::shared_ptr<InstrumentList::Content>> summarizeContent() const;
	
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
		QString __path;					///< absolute drumkit path
		QString __name;					///< drumkit name
		QString __author;				///< drumkit author
		QString __info;					///< drumkit free text
		License __license;				///< drumkit license description
		QString __image;				///< drumkit image filename
		License __imageLicense;			///< drumkit image license

		bool __samples_loaded;			///< true if the instrument samples are loaded
		InstrumentList* __instruments;  ///< the list of instruments
		std::vector<DrumkitComponent*>* __components;  ///< list of drumkit component

		/*
		 * save the drumkit within the given XMLNode
		 * \param node the XMLNode to feed
		 * \param component_id to chose the component to save or -1 for all
		 * \param bRecentVersion Whether the drumkit format should be
		 * supported by Hydrogen 0.9.7 or higher (whether it should be
		 * composed of DrumkitComponents).
		 */
	void save_to( XMLNode* node, int component_id=-1, bool bRecentVersion = true ) const;
		/**
		 * load a drumkit from an XMLNode
		 * \param node the XMLDode to read from
		 * \param dk_path the directory holding the drumkit data
		 * \param bSilent if set to true, all log messages except of
		 * errors and warnings are suppressed.
		 * \param lookup Where to search (system/user folder or both)
		 * for the drumkit.
		 */
	static Drumkit* load_from( XMLNode* node,
							   const QString& dk_path,
							   bool bSilent = false,
							   Filesystem::Lookup lookup = Filesystem::Lookup::stacked );

	/**
	 * Loads the drumkit stored in @a sDrumkitDir into @a pDoc and
	 * takes care of all the error handling.
	 *
	 * \return true on success.
	 */
	static bool loadDoc( const QString& sDrumkitDir, XMLDoc* pDoc, bool bSilent = false );

};

// DEFINITIONS

inline InstrumentList* Drumkit::get_instruments() const
{
	return __instruments;
}

inline void Drumkit::set_path( const QString& path )
{
	__path = path;
}

inline const QString& Drumkit::get_path() const
{
	return __path;
}

inline void Drumkit::set_name( const QString& name )
{
	__name = name;
}

inline const QString& Drumkit::get_name() const
{
	return __name;
}

inline void Drumkit::set_author( const QString& author )
{
	__author = author;
	__license.setCopyrightHolder( author );
	__imageLicense.setCopyrightHolder( author );
}

inline const QString& Drumkit::get_author() const
{
	return __author;
}

inline void Drumkit::set_info( const QString& info )
{
	__info = info;
}

inline const QString& Drumkit::get_info() const
{
	return __info;
}

inline void Drumkit::set_license( const License& license )
{
	__license = license;
}

inline const License& Drumkit::get_license() const
{
	return __license;
}

inline void Drumkit::set_image( const QString& image )
{
	__image = image;
}

inline const QString& Drumkit::get_image() const
{
	return __image;
}

inline void Drumkit::set_image_license( const License& imageLicense )
{
	__imageLicense = imageLicense;
}

inline const License& Drumkit::get_image_license() const
{
	return __imageLicense;
}

inline const bool Drumkit::samples_loaded() const
{
	return __samples_loaded;
}

inline std::vector<DrumkitComponent*>* Drumkit::get_components()
{
	return __components;
}

};

#endif // H2C_DRUMKIT_H

/* vim: set softtabstop=4 noexpandtab: */
