
/*  © Copyright CERN, 2015. All rights not expressly granted are reserved.

    The stub of this file was generated by quasar (https://github.com/quasar-team/quasar/)

    Quasar is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public Licence as published by
    the Free Software Foundation, either version 3 of the Licence.
    Quasar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public Licence for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Quasar.  If not, see <http://www.gnu.org/licenses/>.


 */


#ifndef __DSdoValidator__H__
#define __DSdoValidator__H__

#include <Base_DSdoValidator.h>
#include <muParser.h> // move to the body.

namespace Device
{

class DNode;

class
    DSdoValidator
    : public Base_DSdoValidator
{

public:
    /* sample constructor */
    explicit DSdoValidator (
        const Configuration::SdoValidator& config,
        Parent_DSdoValidator* parent
    ) ;
    /* sample dtr */
    ~DSdoValidator ();

    /* delegators for
    cachevariables and sourcevariables */


    /* delegators for methods */
    UaStatus callValidate (
        OpcUa_Boolean& passed
    ) ;

private:
    /* Delete copy constructor and assignment operator */
    DSdoValidator( const DSdoValidator& other );
    DSdoValidator& operator=(const DSdoValidator& other);

    // ----------------------------------------------------------------------- *
    // -     CUSTOM CODE STARTS BELOW THIS COMMENT.                            *
    // -     Don't change this comment, otherwise merge tool may be troubled.  *
    // ----------------------------------------------------------------------- *

public:
    void initialize(DNode* node);
    bool validate();

private:
    DNode* m_node;



};

}

#endif // __DSdoValidator__H__