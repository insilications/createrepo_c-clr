/* createrepo_c - Library of routines for manipulation with repodata
 * Copyright (C) 2013  Tomas Mlcoch
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

#ifndef __C_CREATEREPOLIB_XML_PARSER_INTERNAL_H__
#define __C_CREATEREPOLIB_XML_PARSER_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <expat.h>
#include "xml_parser.h"
#include "error.h"
#include "package.h"

#define XML_BUFFER_SIZE         8192
#define CONTENT_REALLOC_STEP    256

/* Some notes about XML parsing (primary, filelists, other)
 * ========================================================
 * - Error during parsing is indicated via cr_ParserData->err member.
 * - User specified callback have to be sanitized! User callbacks
 *   are allowed return CR_CB_RET_ERR and do not set the GError.
 *   So if the CR_CB_RET_ERR is returned and GError not setted, caller
 *   of the callback has to set the GError by himself.
 */

/* File types in filelists.xml */
typedef enum {
    FILE_FILE,
    FILE_DIR,
    FILE_GHOST,
    FILE_SENTINEL,
} cr_FileType;

typedef struct {
    unsigned int    from;       /*!< State (current tag) */
    char            *ename;     /*!< String name of sub-tag */
    unsigned int    to;         /*!< State of sub-tag */
    int             docontent;  /*!< Read text content of element? */
} cr_StatesSwitch;

typedef struct _cr_ParserData {
    int          depth;
    int          statedepth;
    unsigned int state;      /*!< current state */
    GError       *err;       /*!< Error message */


    /* Tag content related values */

    int     docontent;  /*!< Store text content of the current element? */
    char    *content;   /*!< Text content of the element */
    int     lcontent;   /*!< The content lenght */
    int     acontent;   /*!< Available bytes in the content */

    XML_Parser      *parser;    /*!< The parser */
    cr_StatesSwitch **swtab;    /*!< Pointers to statesswitches table */
    unsigned int    *sbtab;     /*!< stab[to_state] = from_state */

    /* Package stuff */

    void                    *newpkgcb_data;    /*!<
        User data for the newpkgcb. */
    cr_XmlParserNewPkgCb    newpkgcb;           /*!<
        Callback called to get (create new or use existing from a previous
        parsing of other or primary xml file) pkg object for the currently
        loaded pkg. */
    void                    *pkgcb_data;        /*!<
        User data for the pkgcb. */
    cr_XmlParserPkgCb       pkgcb;              /*!<
        Callback called when a signel pkg data are completly parsed. */
    void                    *warningcb_data;    /*!<
        User data fot he warningcb. */
    cr_XmlParserWarningCb   warningcb;
    cr_Package              *pkg;               /*!<
        The package which is currently loaded. */

    /* Primary related stuff */

    int do_files;   /*!<
        If == 0 then parser will ignore files elements in the primary.xml.
        This is useful when you are inteding parse primary.xml as well as
        filelists.xml. In this case files will be filled from filelists.xml.
        If you are inteding parse only the primary.xml then it coud be useful
        to parse files in primary.
        If you parse files from both a primary.xml and a filelists.xml
        then some files in package object will be duplicated! */

    /* Filelists + Primary related stuff */

    cr_FileType last_file_type;

    /* Other related stuff */

    cr_ChangelogEntry *changelog;

} cr_ParserData;

/** Malloc and initialize common part of XML parser data.
 */
cr_ParserData *cr_xml_parser_data();

/** Frees XML parser data.
 */
void cr_xml_parser_data_free(cr_ParserData *pd);

/** Find attribute in list of attributes.
 * @param name      Attribute name.
 * @param attr      List of attributes of the tag
 * @return          Value or NULL
 */
static inline const char *
cr_find_attr(const char *name, const char **attr)
{
    while (*attr) {
        if (!strcmp(name, *attr))
            return attr[1];
        attr += 2;
    }

    return NULL;
}

/** XML character handler
 */
void XMLCALL cr_char_handler(void *pdata, const XML_Char *s, int len);

/** Wrapper for user warning cb.
 * It checks if warningcb is defined, if defined, it build warning msg from
 * va_args, calls warningcb and propagate (set) error if necessary.
 */
int cr_xml_parser_warning(cr_ParserData *pd,
                          cr_XmlParserWarningType type,
                          const char *msg,
                          ...);

/** strtoll with ability to call warning cb if error during conversion.
 */
gint64 cr_xml_parser_strtoll(cr_ParserData *pd,
                             const char *nptr,
                             unsigned int base);

/** Default callback for the new package.
 */
int cr_newpkgcb(cr_Package **pkg,
                const char *pkgId,
                const char *name,
                const char *arch,
                void *cbdata,
                GError **err);

/** Generic parser.
 */
int
cr_xml_parser_generic(XML_Parser parser,
                      cr_ParserData *pd,
                      const char *path,
                      GError **err);

#ifdef __cplusplus
}
#endif

#endif /* __C_CREATEREPOLIB_XML_PARSER_INTERNAL_H__ */