#ifndef SAPI_INET_WIFI_HPP
#define SAPI_INET_WIFI_HPP

#if !defined __link

#include <sos/api/wifi_api.h>

#include "../api/InetObject.hpp"
#include "../chrono/MicroTime.hpp"
#include "../chrono/Timer.hpp"
#include "../var/String.hpp"
#include "IpAddress.hpp"

namespace inet {

typedef api::Api<wifi_api_t, WIFI_API_REQUEST> WifiApi;

class WifiSsidInfo {
public:
	WifiSsidInfo(){
		m_info = {0};
	}
	explicit WifiSsidInfo(const wifi_ssid_info_t & info) : m_info(info) {}

	bool is_valid() const {
		return m_info.ssid[0] != 0;
	}

	var::String get_name() const {
		return var::String(m_info.ssid);
	}

	WifiSsidInfo& set_name(const var::String & value){
		strncpy(m_info.ssid, value.cstring(), sizeof(m_info.ssid));
		return *this;
	}


	API_ACCESS_MEMBER_FUNDAMENTAL(WifiSsidInfo,u8,info,channel)
	API_ACCESS_MEMBER_FUNDAMENTAL(WifiSsidInfo,u8,info,security)
	API_ACCESS_MEMBER_FUNDAMENTAL(WifiSsidInfo,s8,info,rssi)

	const wifi_ssid_info_t & info() const {
		return m_info;
	}

	bool operator == (const WifiSsidInfo & a) const {
		return a.get_name() == get_name();
	}

private:
	wifi_ssid_info_t m_info;
};

class WifiAuthInfo {
public:

	WifiAuthInfo(){
		m_auth ={0};
	}

	explicit WifiAuthInfo(const wifi_auth_info_t & auth) : m_auth(auth) {}

	explicit WifiAuthInfo(const var::String& passphrase){
		strncpy(
					(char*)(m_auth.password),
					passphrase.cstring(),
					sizeof(m_auth.password)
					);
	}


	const wifi_auth_info_t & auth() const {
		return m_auth;
	}

private:
	wifi_auth_info_t m_auth;
};

class WifiScanAttributes {
public:

	enum scan_regions {
		scan_region_north_america = WIFI_SCAN_REGION_NORTH_AMERICA,
		scan_region_asia = WIFI_SCAN_REGION_ASIA
	};

	WifiScanAttributes(){
		m_attributes = {0};
	}

	static WifiScanAttributes get_default(){
		return WifiScanAttributes()
				.set_region(scan_region_north_america)
				.set_passive(false)
				.set_channel(0xff)
				.set_slot_count(5)
				.set_slot_time(chrono::Milliseconds(100))
				.set_probe_count(2)
				.set_rssi_threshold(-90);
	}

	explicit WifiScanAttributes(const wifi_scan_attributes_t & attributes) : m_attributes(attributes){}

	bool is_valid() const {
		return m_attributes.slot_count != 0;
	}

	WifiScanAttributes& set_passive(bool value = true){
		m_attributes.is_passive = value;
		return *this;
	}

	WifiScanAttributes& set_slot_time(const chrono::MicroTime & value){
		m_attributes.slot_time_ms = value.milliseconds();
		return *this;
	}

	WifiScanAttributes& set_region(enum scan_regions value){
		m_attributes.scan_region = value;
		return *this;
	}

	chrono::MicroTime slot_time() const {
		return chrono::Milliseconds(m_attributes.slot_time_ms);
	}

	API_ACCESS_MEMBER_FUNDAMENTAL(WifiScanAttributes,u8,attributes,channel)
	API_ACCESS_MEMBER_FUNDAMENTAL(WifiScanAttributes,u8,attributes,slot_count)
	API_ACCESS_MEMBER_FUNDAMENTAL(WifiScanAttributes,u8,attributes,probe_count)
	API_ACCESS_MEMBER_FUNDAMENTAL(WifiScanAttributes,u8,attributes,rssi_threshold)
	API_READ_ACCESS_MEMBER_FUNDAMENTAL(WifiScanAttributes,u8,attributes,scan_region)

	bool is_passive() const {
		return m_attributes.is_passive;
	}

	const wifi_scan_attributes_t & attributes() const {
		return m_attributes;
	}

private:
	wifi_scan_attributes_t m_attributes;
};

class WifiIpInfo {
public:
	WifiIpInfo(){ m_info = {0}; }
	explicit WifiIpInfo(const wifi_ip_info_t & info) : m_info(info){}

	bool is_valid() const {
		return m_info.ip_address != 0;
	}


	WifiIpInfo& set_lease_time(const chrono::MicroTime & value){
		m_info.lease_time_s = value.seconds();
		return *this;
	}

	chrono::MicroTime lease_time() const {
		return chrono::Seconds(m_info.lease_time_s);
	}

	API_ACCESS_MEMBER_FUNDAMENTAL(WifiIpInfo,u32,info,ip_address)
	API_ACCESS_MEMBER_FUNDAMENTAL(WifiIpInfo,u32,info,dns_address)
	API_ACCESS_MEMBER_FUNDAMENTAL(WifiIpInfo,u32,info,subnet_mask)
	API_ACCESS_MEMBER_FUNDAMENTAL(WifiIpInfo,u32,info,gateway_address)

	Ipv4Address get_ip_address() const {
		return Ipv4Address(ip_address());
	}

	Ipv4Address get_dns_address() const {
		return Ipv4Address(dns_address());
	}

	Ipv4Address get_gateway_address() const {
		return Ipv4Address(gateway_address());
	}

	Ipv4Address get_subnet_mask() const {
		return Ipv4Address(subnet_mask());
	}


private:
	wifi_ip_info_t m_info;
};

class WifiInfo {
public:
	WifiInfo(){ m_info = {0}; }
	WifiInfo(const wifi_info_t & info){ m_info = info; }

	bool is_valid() const {
		return m_info.resd0 == WIFI_API_INFO_RESD;
	}

	WifiIpInfo get_ip_info() const{
		return WifiIpInfo(m_info.ip);
	}

	API_ACCESS_MEMBER_FUNDAMENTAL(WifiInfo,u8,info,security)
	API_ACCESS_MEMBER_FUNDAMENTAL(WifiInfo,u8,info,rssi)

	bool is_connected() const {
		return m_info.is_connected;
	}

	const wifi_info_t & info() const {
		return m_info;
	}

private:
	wifi_info_t m_info;
};


class Wifi : public api::WorkObject, public WifiApi {
public:
	Wifi();
	~Wifi(){
		finalize();
	}

	int initialize(){
		if( api().is_valid() == false ){
			return set_error_number_if_error(api::error_code_inet_wifi_api_missing);
		}
		return set_error_number_if_error( api()->init(&m_context) );
	}

	void finalize(){
		api()->deinit(&m_context);
	}

	WifiIpInfo connect(
			const WifiSsidInfo & ssid_info,
			const WifiAuthInfo & auth,
			const chrono::MicroTime & timeout = chrono::Seconds(10)
			);

	int disconnect(){
		return set_error_number_if_error(
					api()->disconnect(m_context)
					);
	}

	var::Vector<WifiSsidInfo> scan(
			const WifiScanAttributes & attributes = WifiScanAttributes::get_default(),
			const chrono::MicroTime & timeout = chrono::Seconds(20)
			);


	int start_scan(
			const WifiScanAttributes & attributes
			){
		return set_error_number_if_error(
					api()->start_scan(
						m_context,
						&attributes.attributes()
						)
					);
	}


	bool is_scan_busy() const {
		int result
				= api()->get_scan_count(m_context);
		return result < 0;
	}

	WifiInfo get_info(){
		wifi_info_t info;
		if( set_error_number_if_error(
					api()->get_info(
						m_context,
						&info
						)
					) < 0 ){
			return WifiInfo();
		}
		return WifiInfo(info);
	}

	var::Vector<WifiSsidInfo> get_ssid_info_list();

	int set_mode();
	int set_mac_address(u8 mac_address[6]);
	int get_mac_address(u8 mac_address[6]);
	int get_factory_mac_address(u8 mac_address[6]);
	int set_ip_address(
			const wifi_ip_info_t * static_ip_address
			);

	int set_sleep_mode(void * context);
	int sleep(void * context, u32 sleep_time_ms);
	int set_device_name(
			void * context,
			const char * name
			);
	int set_tx_power(
			void * context,
			u8 power_level
			);

	static WifiApi & api(){ return m_api; }

private:
	static WifiApi m_api;
	void * m_context;

};

}

#endif

#endif // SAPI_INET_WIFI_HPP
