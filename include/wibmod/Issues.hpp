/**
 * @file Issues.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef WIBMOD_INCLUDE_WIBMOD_ISSUES_HPP_
#define WIBMOD_INCLUDE_WIBMOD_ISSUES_HPP_

#include "appfwk/DAQModule.hpp"
#include "ers/Issue.hpp"

#include <string>

// NOLINTNEXTLINE(build/define_used)
#define TLVL_ENTER_EXIT_METHODS 10
// NOLINTNEXTLINE(build/define_used)
#define TLVL_GENERATION 11
// NOLINTNEXTLINE(build/define_used)
#define TLVL_CANDIDATE 15

namespace dunedaq {

ERS_DECLARE_ISSUE_BASE(wibmod,
                       UnreachableError,
                       appfwk::GeneralDAQModuleIssue,
                       "An unreachable part of the code has been reached.",
                       ((std::string)name),
                       ERS_EMPTY)
                       
ERS_DECLARE_ISSUE_BASE(wibmod,
                       ConfigurationFailed,
                       appfwk::GeneralDAQModuleIssue,
                       "WIB " << name << " failed to configure.",
                       ((std::string)name),
                       ERS_EMPTY)

} // namespace dunedaq

#endif // WIBMOD_INCLUDE_WIBMOD_ISSUES_HPP_
