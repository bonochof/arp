bool get_ip_address(char* addr, in_addr_t* res);
bool get_device_ip_address(int soc, char* device, in_addr_t* res);
bool get_device_mac_address(int soc, char* device, u_char res[6]);