/**

Dual license
 - GNU Affero GPL v3 (https://opensource.org/licenses/agpl-v3.html)

For commercial license please contact Michal Pomaranski (mpomaranski@gmail.com)

*/

#ifndef YGSMMODEM_POWER_CONTROL_HAS_BEEN_INCLUDED
#define YGSMMODEM_POWER_CONTROL_HAS_BEEN_INCLUDED


#ifdef __cplusplus
extern "C" {
#endif

struct ygsmmodem_power_control;
typedef struct ygsmmodem_power_control * ygsmmodem_power_control_t;

/**
\brief Defines the data source used by physical layer to send the data. It should
write required commands to the serial and exit after sending the command

The physical layer takes care of ACK
*/
typedef (void)(*ygsmmodem_data_source_fn)(yserial_t serial);
typedef (void)(*ygsmmodem_data_sink_fn)(yserial_t serial);

/**
\brief Creates new instance of physical layer

\param serial connection used to read/write data over Sim900 protocol
\param sink function which will receive data, stripped from power control data
\return new instance of physical layer
*/
ygsmmodem_power_control_t ygsmmodem_power_control_alloc(yserial_t serial, ygsmmodem_data_sink_fn sink);

/**
\brief Frees memory allocated to the physical layer

\param power_control to be freed
*/
void ygsmmodem_power_control_free(ygsmmodem_power_control_t power_control);

/**
\brief Used to send data over the wire. The physical layer will make sure the power is on and then call source for transmitting the data

\param power_control instance
\param source of the data

\returns the result if the operation was scheduled
*/
yerr_t ygsmmodem_power_control_send_data(ygsmmodem_power_control_t power_control, ygsmmodem_data_source_fn source);

/**
\brief Asks the layer to enter the power save mode

\returns if request can be completed
*/
yerr_t ygsmmodem_power_control_power_save();

#ifdef __cplusplus
}
#endif

#endif
