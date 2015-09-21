// Simple module that registers bluetooth connect services and sets up a switch of icons
static void bt_handler(bool connected) {
	if (connected) {
		bitmap_layer_set_bitmap(s_btIcon_layer, s_btIconOn_bitmap);		// display the connected image
	} else {
		bitmap_layer_set_bitmap(s_btIcon_layer, s_btIconOff_bitmap);	// display the unconnected image
	}
}
static void btModule_init() {
	bluetooth_connection_service_subscribe(bt_handler);		// Register to the service
}
static void btModule_deinit() {
	bluetooth_connection_service_unsubscribe();		// Deregister the service
}