/*
 * service.h
 *
 *  Created on: Jul 12, 2013
 *      Author: wiselib
 */

#ifndef SERVICE_H_
#define SERVICE_H_

// TODO this should be a concept
// TODO reformat comments (borrowed from nCoAp implementation)

namespace wiselib
{
/**
 * \brief This is the interface to be implemented to realize a CoAP webservice.
 * The generic type Value_P means, that the object that holds the status of the resource is of type Value_P.
 *
 */
template<typename Value_P, typename Message_P, typename String_T>
class IService
{
public:
	typedef Value_P value_t;
	typedef String_T string_t;
	typedef Message_P message_t;
	typedef IService<value_t, message_t, string_t> self_t;

	virtual ~IService() {}

    /**
     * Returns the (relative) path this service is listening at
     * @return relative path of the service (e.g. /path/to/service)
     */
     virtual string_t path() = 0;

    /**
     * Returns the object of type T that holds the actual status of the resource represented by this
     * {@link NotObservableWebService}.
     *
     * Note, that this status is internal and thus independent from the payload of the {@link CoapResponse} to be
     * computed by the inherited method {@link #processCoapRequest(SettableFuture, CoapRequest, InetSocketAddress)}.
     *
     * Example: Assume this webservice represents a switch that has two states "on" and "off". The payload of the
     * previously mentioned {@link CoapResponse} could then be either "on" or "off". But since there are only
     * two possible states {@link T} could be of type {@link Boolean}.
     *
     * @return the object of type T that holds the actual resourceStatus of the resource
     */
     virtual value_t status() = 0;

    /**
     * Method to set the new status of the resource represented by this {@link WebService}. This method is the
     * one and only recommended way to change the status.
     *
     * Note, that this status is internal and thus independent from the payload of the {@link CoapResponse} to be
     * returned by the inherited method {@link #processCoapRequest(SettableFuture, CoapRequest, InetSocketAddress)}.
     *
     * Example: Assume this webservice represents a switch that has two states "on" and "off". The payload of the
     * previously mentioned {@link CoapResponse} could then be either "on" or "off". But since there are only
     * two possible states {@link T} could be of type {@link Boolean}.
     *
     * @param newStatus the object of type {@link T} representing the new status
     */
     virtual void set_status(value_t newStatus) = 0;

    /**
     * The max-age value represents the validity period (in seconds) of the actual status. The nCoap framework uses this
     * value to set the {@link OptionName#MAX_AGE} wherever necessary or useful. The framework does not change or remove
     * manually set max-age options in {@link CoapResponse} instances, i.e. using {@code response.setMaxAge(int)}.
     *
     * @return the max-age value of this {@link WebService} instance. If not set to another value implementing classes must
     * return {@link OptionRegistry#MAX_AGE_DEFAULT} as default value.
     */
     virtual uint32_t max_age() = 0;


    /**
     * This method is called by the nCoAP framework when this {@link WebService} is removed from the
     * {@link CoapServerApplication} instance. If any one could e.g. try to cancel scheduled tasks. There might even
     * be no need to do anything at all, i.e. implement the method with empty body.
     *
     * If this {@link WebService} uses the default {@link ScheduledExecutorService} to execute tasks one MUST NOT
     * terminate this {@link ScheduledExecutorService} but only cancel scheduled tasks using there
     * {@link ScheduledFuture}.
     */
     virtual void shutdown() = 0;

    /**
     * Method to process an incoming {@link CoapRequest} asynchronously. The implementation of this method is dependant
     * on the concrete webservice. Processing a message might cause a new status of the resource or even the deletion
     * of the complete resource, i.e. this {@link WebService} instance.
     *
     * Implementing classes have to make sure that {@link SettableFuture<CoapResponse>#set(CoapResponse)} is invoked
     * after some time. Otherwise the {@link CoapServerApplication} will wait forever, even though non-blocking.
     *
     * The way to process the incoming request is basically to be implemented based on the {@link Code},
     * the {@link MsgType}, the contained {@link Option}s (if any) and (if any) the payload of the request.
     *
     * @param responseFuture the {@link SettableFuture} instance to set the {@link CoapResponse} which is the result
     *                       of the incoming {@link CoapRequest}. Use
     *                       {@link SettableFuture<CoapResponse>#set(CoapResponse)} to send it to the client.
     * @param request The {@link CoapRequest} to be processed by the {@link WebService} instance
     * @param remoteAddress The address of the sender of the request
     *
     */
    //void processCoapRequest(SettableFuture<CoapResponse> responseFuture, CoapRequest request,
    //                               InetSocketAddress remoteAddress);
     template <class T, void (T::*TMethod)( typename self_t::message_t & ) >
     int reg_request_callback( T *callback );

};

}
#endif /* SERVICE_H_ */
