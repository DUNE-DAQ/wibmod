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
#include <iomanip>

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
                               
// WIB1 Issues

ERS_DECLARE_ISSUE_BASE(wibmod,
                       InvalidPartitionNumber,
                       appfwk::GeneralDAQModuleIssue,
                       "partition_number must be 0-15, not: " << partition_number,
                       ((std::string)name),
                       ((int)partition_number))

ERS_DECLARE_ISSUE_BASE(wibmod,
                       WrongFirmwareMode,
                       appfwk::GeneralDAQModuleIssue,
                       "WIB Firmware setup in " << fw_mode << " readout mode but configure expects " << fw_expect,
                       ((std::string)name),
                       ((std::string)fw_mode) ((std::string)fw_expect) )

ERS_DECLARE_ISSUE_BASE(wibmod,
                       UnknownFirmwareMode,
                       appfwk::GeneralDAQModuleIssue,
                       "WIB Firmware setup in unknown readout mode",
                       ((std::string)name),
                       ERS_EMPTY)

ERS_DECLARE_ISSUE_BASE(wibmod,
                       IncorrectFirmwareVersion,
                       appfwk::GeneralDAQModuleIssue,
                       type << " firmware version is "
                            << std::hex << std::setw(8) << std::setfill('0')
                            << fw_version
                            <<" but expected "
                            << std::hex << std::setw(8) << std::setfill('0')
                            << expected_fw_version
                            <<" version",
                       ((std::string)name),
                       ((std::string)type) ((uint32_t)fw_version) ((uint32_t)expected_fw_version))

ERS_DECLARE_ISSUE_BASE(wibmod,
                       BadDuplicationMode,
                       appfwk::GeneralDAQModuleIssue,
                       "WIB is set to duplicate data from links 1 and 2 to 3 and 4. This shouldn't happen!",
                       ((std::string)name),
                       ERS_EMPTY)
                       
ERS_DECLARE_ISSUE_BASE(wibmod,
                       WIBCommunicationError,
                       appfwk::GeneralDAQModuleIssue,
                       "WIB communication error:" << what,
                       ((std::string)name),
                       ((std::string)what))

ERS_DECLARE_ISSUE_BASE(wibmod,
                       UnhandledBUException,
                       appfwk::GeneralDAQModuleIssue,
                       "Unhandled BUException: " << what << " : " << description,
                       ((std::string)name),
                       ((std::string)what) ((std::string)description))
                       
ERS_DECLARE_ISSUE_BASE(wibmod,
                       InvalidFEMBSetting,
                       appfwk::GeneralDAQModuleIssue,
                       message,
                       ((std::string)name),
                       ((std::string)message))
                       
ERS_DECLARE_ISSUE_BASE(wibmod,
                       CommandFailed,
                       appfwk::GeneralDAQModuleIssue,
                       message,
                       ((std::string)name),
                       ((std::string)message))
                       
ERS_DECLARE_ISSUE_BASE(wibmod,
                       CannotReadFromFEMB,
                       appfwk::GeneralDAQModuleIssue,
                       message,
                       ((std::string)name),
                       ((std::string)message))

} // namespace dunedaq

#endif // WIBMOD_INCLUDE_WIBMOD_ISSUES_HPP_
