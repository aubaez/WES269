#include <stdbool.h>
#include <zephyr/types.h>
#include <zephyr/drivers/sensor.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

//adding header files for gpio
#include <stdio.h>
#include <zephyr/drivers/gpio.h>

#define LAB2_SERVICE_UUID BT_UUID_128_ENCODE(0xBDFC9792, 0x8234, 0x405E, 0xAE02, 0x35EF4174B299)

static ssize_t characteristic_read(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);

//reference format to bt_gatt_attr_write_ccc()
static ssize_t characteristic_write(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset, uint8_t flags);

// Global value that saves state for the characteristic.
uint16_t characteristic_value = 0x001;


/***********************************************/
// LED Parameters (reference to Blinky code)   //
/***********************************************/
/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000
/* The devicetree node identifier for the "led1" alias. */
#define LED1_NODE DT_ALIAS(led1)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
/***********************************************/

// Set up the advertisement data.
#define DEVICE_NAME "MyDeviceLB"
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

static const struct bt_data ad[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, LAB2_SERVICE_UUID)
};

// Setup the the service and characteristics.
BT_GATT_SERVICE_DEFINE(lab2_service,
    BT_GATT_PRIMARY_SERVICE(
        BT_UUID_DECLARE_128(LAB2_SERVICE_UUID)
    ),
    BT_GATT_CHARACTERISTIC(BT_UUID_DECLARE_16(0x0001), BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
                   BT_GATT_PERM_READ | BT_GATT_PERM_WRITE, characteristic_read, characteristic_write, &characteristic_value),
);


// Callback when a client reads the characteristic.
//
// Documented under name "bt_gatt_attr_read_chrc()"
static ssize_t characteristic_read(struct bt_conn *conn,
                                   const struct bt_gatt_attr *attr,
                                   void *buf,
                                   uint16_t len,
                                   uint16_t offset)
{
    // Need to encode data into a buffer to send to client.
    uint8_t out_buffer[4] = {0};

    //out_buffer[0]=0x00;
    //out_buffer[1]=0x00;
    //out_buffer[2]=0x00;
    //out_buffer[3]= 0x01;
 
    // User helper function to encode the output data to send to
    // the client.
    return bt_gatt_attr_read(conn, attr, buf, len, offset, out_buffer, 4);
}

// Documented under name "bt_gatt_attr_write_ccc()"
static ssize_t characteristic_write(struct bt_conn *conn,
                                   const struct bt_gatt_attr *attr,
                                   void *buf,
                                   uint16_t len,
                                   uint16_t offset,
                                   uint8_t flags){
    int ret;
    ret = gpio_pin_toggle_dt(&led); //toggle LED
    if(ret){
        printk("Write Error. \n");
    }
    return len;
}

// Setup callbacks when devices connect and disconnect.
static void connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        printk("Connection failed (err 0x%02x)\n", err);
    } else {
        printk("Connected\n");
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    printk("Disconnected (reason 0x%02x)\n", reason);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};


static void bt_ready(int err)
{
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    printk("Bluetooth initialized\n");

    err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        printk("Advertising failed to start (err %d)\n", err);
        return;
    }

    printk("Advertising successfully started\n");
}

void main(void)
{
    int err, ret;

    err = bt_enable(bt_ready);
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    if (!gpio_is_ready_dt(&led)) {
        return 0;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE); //start LED off
    if (ret < 0) {
        return 0;
    }
}

