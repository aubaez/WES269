#include <zephyr/types.h>
#include <stddef.h>
#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/byteorder.h>

//adding header files for gpio
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

#define LAB2_SERVICE_UUID BT_UUID_128_ENCODE(0xBDFC9792, 0x8234, 0x405E, 0xAE02, 0x35EF4174B299)
#define LAB2_SERVICE_CHARACTERISTIC_UUID 0x0001

static void start_scan(void);

static struct bt_conn *default_conn;

static struct bt_uuid* search_service_uuid = BT_UUID_DECLARE_128(LAB2_SERVICE_UUID);
static struct bt_uuid* search_characteristic_uuid = BT_UUID_DECLARE_16(LAB2_SERVICE_CHARACTERISTIC_UUID);
static struct bt_gatt_discover_params discover_params;
static struct bt_gatt_read_params read_params;
static struct bt_gatt_write_params write_params;  //adding write functionality

//defining button parameters (looking at button sample code)
#define SW0_NODE	DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS_OKAY(SW0_NODE)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios,{0});
static struct gpio_callback button_cb_data;

//define set led param
static uint8_t ledSet = 0x01;

// Callback after reading characteristic value.
static uint8_t read_func(struct bt_conn *conn, uint8_t err,
                   struct bt_gatt_read_params *params,
                   const void *data, uint16_t length)
{
    if (err) {
        printk("read failed (err %d)\n", err);
    }

    uint8_t* buf = (uint8_t*) data;

    if (length == 4) {
        uint32_t val = (((uint32_t) buf[0]) << 24) |
                       (((uint32_t) buf[1]) << 16) |
                       (((uint32_t) buf[2]) <<  8) |
                       (((uint32_t) buf[3]) <<  0);

        printk("Read: 0x%x\n", val);

    }

    return BT_GATT_ITER_STOP;
}

// Callback after writing characteristic value.
static uint8_t write_func(struct bt_conn *conn, uint8_t err,
                   struct bt_gatt_write_params *params,
                   const void *data, uint16_t length)
{
    if (err) {
        printk("write to LED failed (err %d)\n", err);
    }

    return BT_GATT_ITER_STOP;
}

static uint8_t discover_func(struct bt_conn *conn,
                 const struct bt_gatt_attr *attr,
                 struct bt_gatt_discover_params *params)
{
    int err;

    if (!attr) {
        printk("Discover complete\n");
        (void)memset(params, 0, sizeof(*params));
        return BT_GATT_ITER_STOP;
    }

    // printk("[ATTRIBUTE] handle %u\n", attr->handle);

    if (bt_uuid_cmp(discover_params.uuid, BT_UUID_DECLARE_128(LAB2_SERVICE_UUID)) == 0) {
        // printk("Found service\n");

        discover_params.uuid = search_characteristic_uuid;
        discover_params.start_handle = attr->handle + 1;
        discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;

        err = bt_gatt_discover(conn, &discover_params);
        if (err) {
            printk("Discover failed (err %d)\n", err);
        }
    }
    else if (bt_uuid_cmp(discover_params.uuid, BT_UUID_DECLARE_16(LAB2_SERVICE_CHARACTERISTIC_UUID)) == 0) {
        printk("Found characteristic\n");

        read_params.func = read_func;
        read_params.handle_count = 1;
        read_params.single.handle = bt_gatt_attr_value_handle(attr);
        read_params.single.offset = 0U;

        //save the uint16_t values returned from bt_gatt_attr_value_handle() to read from the characteristic at a later time.	
        write_params.handle = bt_gatt_attr_value_handle(attr); // Note: can only get characteristic handle at discovery

        err = bt_gatt_read(conn, &read_params);
        if (err) {
            printk("Read failed (err %d)\n", err);
        }
    }

    return BT_GATT_ITER_STOP;
}

static void connected(struct bt_conn *conn, uint8_t conn_err)
{
    char addr[BT_ADDR_LE_STR_LEN];
    int err;

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
    if (conn_err) {
        printk("Failed to connect to %s (%u)\n", addr, conn_err);

        bt_conn_unref(default_conn);
        default_conn = NULL;

        start_scan();
        return;
    }

    printk("Connected: %s\n", addr);

    if (conn == default_conn) {
        discover_params.uuid = search_service_uuid;
        discover_params.func = discover_func;
        discover_params.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE;
        discover_params.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE;
        discover_params.type = BT_GATT_DISCOVER_PRIMARY;

        // call write function while connection exists
        write_params.func = write_func;
        write_params.offset = 0;
        write_params.data = &ledSet;
        write_params.length = sizeof(ledSet);
        err = bt_gatt_discover(default_conn, &discover_params);
        if (err) {
            printk("Discover failed(err %d)\n", err);
            return;
        }
    }

}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    printk("Disconnected: %s (reason 0x%02x)\n", addr, reason);

    if (default_conn != conn) {
        return;
    }

    bt_conn_unref(default_conn);
    default_conn = NULL;

    start_scan();
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};

// Called for each advertising data element in the advertising data.
static bool ad_found(struct bt_data *data, void *user_data)
{
    bt_addr_le_t *addr = user_data;

    printk("[AD]: %u data_len %u\n", data->type, data->data_len);

    switch (data->type) {
    case BT_DATA_UUID128_ALL:
        if (data->data_len != 16) {
            printk("AD malformed\n");
            return true;
        }

        struct bt_le_conn_param *param;
        struct bt_uuid uuid;
        int err;

        bt_uuid_create(&uuid, data->data, 16);
        if (bt_uuid_cmp(&uuid, BT_UUID_DECLARE_128(LAB2_SERVICE_UUID)) == 0) {
            printk("Found matching advertisement\n");

            err = bt_le_scan_stop();
            if (err) {
                printk("Stop LE scan failed (err %d)\n", err);
                return false;
            }

            param = BT_LE_CONN_PARAM_DEFAULT;
            err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN, param, &default_conn);
            if (err) {
                printk("Create conn failed (err %d)\n", err);
                start_scan();
            }
        }

        return false;
        
    }

    return true;
}

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
             struct net_buf_simple *ad)
{
    char dev[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(addr, dev, sizeof(dev));
    /*if(strcmp(dev, "E3:5E:02:AA:8F:D7 (random)") == 0){*/
    printk("[DEVICE]: %s, AD evt type %u, AD data len %u, RSSI %i\n",
           dev, type, ad->len, rssi);

    // We're only interested in connectable devices.
    if (type == BT_GAP_ADV_TYPE_ADV_IND ||
        type == BT_GAP_ADV_TYPE_ADV_DIRECT_IND) {
        // Helper function to parse the advertising data (AD) elements
        // from the advertisement. This will call `ad_found()` for
        // each element.
        bt_data_parse(ad, ad_found, (void*) addr);
    }
}

static void start_scan(void)
{
    int err;

    struct bt_le_scan_param scan_param = {
        .type       = BT_LE_SCAN_TYPE_PASSIVE,
        .options    = BT_LE_SCAN_OPT_NONE,
        .interval   = BT_GAP_SCAN_FAST_INTERVAL,
        .window     = BT_GAP_SCAN_FAST_WINDOW,
    };

    err = bt_le_scan_start(&scan_param, device_found);
    if (err) {
        printk("Scanning failed to start (err %d)\n", err);
        return;
    }

    printk("Scanning successfully started\n");
}

static void bt_ready(int err)
{
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    printk("Bluetooth initialized\n");

    start_scan();
}
//reference to button sample code
void buttonPressed(const struct device *dev, struct gpio_callback *cb,
            uint32_t pins){
    printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
	int error = bt_gatt_write_without_response(default_conn, write_params.handle, write_params.data, write_params.length,0);  // call on write function for button press

}
void main(void){
    int err, ret;
    if (!gpio_is_ready_dt(&button)) {
        printk("Error: button device %s is not ready\n",
               button.port->name);
        return;
    }

    ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret != 0) {
        printk("Error %d: failed to configure %s pin %d\n",
               ret, button.port->name, button.pin);
        return;
    }

    ret = gpio_pin_interrupt_configure_dt(&button,
                          GPIO_INT_EDGE_TO_ACTIVE);
    if (ret != 0) {
        printk("Error %d: failed to configure interrupt on %s pin %d\n",
            ret, button.port->name, button.pin);
        return;
    }

    gpio_init_callback(&button_cb_data, buttonPressed, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);
    printk("Set up button at %s pin %d\n", button.port->name, button.pin);

    err = bt_enable(bt_ready);

    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }
}