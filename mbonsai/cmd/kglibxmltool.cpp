/* ////////// LICENSE INFO ////////////////////

 * Copyright (C) 2013 by NYSOL CORPORATION
 *
 * Unless you have received this program directly from NYSOL pursuant
 * to the terms of a commercial license agreement with NYSOL, then
 * this program is licensed to you under the terms of the GNU Affero General
 * Public License (AGPL) as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THOSE OF 
 * NON-INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Please refer to the AGPL (http://www.gnu.org/licenses/agpl-3.0.txt)
 * for more details.

 ////////// LICENSE INFO ////////////////////*/
#include <kgmod.h>
#include <map>
#include <libxml/xmlreader.h>
#include <libxml/xpath.h>
#include "kglibxmltool.h"


map<kgstr_t,kgstr_t> kglibxml::get_attr(xmlNodePtr node){
	map<kgstr_t,kgstr_t> rtn;
	xmlAttrPtr attr = node->properties;
	while(attr){
		rtn[(char*)attr->name] = (char*)attr->children->content;
		attr = attr->next;
	}
	return rtn;
}
