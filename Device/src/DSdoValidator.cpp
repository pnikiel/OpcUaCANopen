
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


#include <Configuration.hxx> // TODO; should go away, is already in Base class for ages

#include <DSdoValidator.h>
#include <ASSdoValidator.h>
#include <DNode.h>
#include <DSdoVariable.h>

#include <Logging.hpp>

using namespace Logging;

namespace Device
{
// 1111111111111111111111111111111111111111111111111111111111111111111111111
// 1     GENERATED CODE STARTS HERE AND FINISHES AT SECTION 2              1
// 1     Users don't modify this code!!!!                                  1
// 1     If you modify this code you may start a fire or a flood somewhere,1
// 1     and some human being may possible cease to exist. You don't want  1
// 1     to be charged with that!                                          1
// 1111111111111111111111111111111111111111111111111111111111111111111111111






// 2222222222222222222222222222222222222222222222222222222222222222222222222
// 2     SEMI CUSTOM CODE STARTS HERE AND FINISHES AT SECTION 3            2
// 2     (code for which only stubs were generated automatically)          2
// 2     You should add the implementation but dont alter the headers      2
// 2     (apart from constructor, in which you should complete initializati2
// 2     on list)                                                          2
// 2222222222222222222222222222222222222222222222222222222222222222222222222

/* sample ctr */
DSdoValidator::DSdoValidator (
    const Configuration::SdoValidator& config,
    Parent_DSdoValidator* parent
):
    Base_DSdoValidator( config, parent),
    m_node(nullptr)

    /* fill up constructor initialization list here */
{
    /* fill up constructor body here */
    
}

/* sample dtr */
DSdoValidator::~DSdoValidator ()
{
}

/* delegates for cachevariables */



/* delegators for methods */
UaStatus DSdoValidator::callValidate (
    OpcUa_Boolean& passed
)
{
    passed = this->validate();
    return OpcUa_Good;
}

// 3333333333333333333333333333333333333333333333333333333333333333333333333
// 3     FULLY CUSTOM CODE STARTS HERE                                     3
// 3     Below you put bodies for custom methods defined for this class.   3
// 3     You can do whatever you want, but please be decent.               3
// 3333333333333333333333333333333333333333333333333333333333333333333333333

void DSdoValidator::initialize(DNode* node)
{
    m_node = node;
}

bool DSdoValidator::validate(std::string* outDescription) // TODO remove out description
{
    mu::Parser parser;
    parser.DefineNameChars("0123456789_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.");
    parser.SetExpr(assertTrueFormula());

    auto usedVariables = parser.GetUsedVar();
    LOG(Log::TRC, "SdoValidator") << wrapId(getFullName()) << " recognized these operands:";
    for (auto& nameValuePair : usedVariables)
    {
        LOG(Log::TRC, "SdoValidator") << wrapId(getFullName()) << " will query SDO named " << nameValuePair.first; // no sense to print the value as it is not initialized yet

        DSdoVariable* sdoVariable (nullptr);
        try
        {
            sdoVariable = m_node->getSdoByShortName(nameValuePair.first); // TODO: protection for not having this short name
        }
        catch (...)
        {
            LOG(Log::ERR, "SdoValidator") << wrapId(getFullName()) << " SDO " << wrapId(nameValuePair.first) << " is not in the short-list (suggests configuratoin file/ model issues?)";
        }
        // try to obtain the value
        UaVariant sdoValueRead;
        UaDateTime sourceTime;
        // TODO: we shall move it to a separate wrapper because of not being able to configure the reply time
        if (!sdoVariable->readValue(sdoValueRead, sourceTime).isGood())
        {
            LOG(Log::ERR, "SdoValidator") << wrapId(getFullName()) << " reading the SDO " << wrapId(nameValuePair.first) << " for validation failed.";
            return false;
        }

        LOG(Log::TRC, "SdoValidator") << wrapId(getFullName()) << " read SDO " << wrapValue(nameValuePair.first) << " value obtained is " << wrapId(sdoValueRead.toString().toUtf8());

        double value;
        if (sdoValueRead.toDouble(value) != OpcUa_Good)
        {
            LOG(Log::ERR, "SdoValidator") << wrapId(getFullName()) << " failure converting the obtained value to a floating-point number.";
            return false;
        }
        parser.DefineConst(nameValuePair.first, value);
    }

    try
    {
        double evaluation = parser.Eval();
        if (evaluation != 0 && evaluation != 1)
            throw std::runtime_error("Validator formula is badly constructed for validator [" + getFullName() + 
                "]. It must evaluate to 1 or 0, i.e. a boolean, it evaluated to [" + std::to_string(evaluation) + "]");
        bool validatedTrue = evaluation == 1.0;
        if (!validatedTrue)
        {
            LOG(Log::ERR, "SdoValidator") << wrapId(getFullName()) << " validated false. This suggests online configuration error: " <<
                wrapValue(description());
        }
        return validatedTrue;
    }
    catch(const mu::Parser::exception_type &e)
    {
        LOG(Log::ERR, "SdoValidator") << wrapId(getFullName()) << " formula evaluation error: " << e.GetMsg();
        return false;
    }

}

}