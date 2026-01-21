import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_THROTTLE
from esphome import automation, pins

DEPENDENCIES = ["uart"]
MULTI_CONF = True

MAX_ZONES = 4

ld6001b_ns = cg.esphome_ns.namespace("ld6001b")
LD6001BComponent = ld6001b_ns.class_("LD6001BComponent", cg.Component, uart.UARTDevice)
Person = ld6001b_ns.struct("Person")
People_t = cg.std_vector.template(Person)
People_t_const_ref = People_t.operator("ref").operator("const")

CONF_LD6001B_ID = "ld6001b_id"
CONF_ON_UPDATE = "on_update"
CONF_RESET_PIN = "reset_pin"

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(LD6001BComponent),
            cv.Optional(CONF_THROTTLE, default="1000ms"): cv.All(
                cv.positive_time_period_milliseconds,
                cv.Range(min=cv.TimePeriod(milliseconds=1)),
            ),
            cv.Optional(CONF_RESET_PIN): pins.internal_gpio_output_pin_schema,

            cv.Optional(CONF_ON_UPDATE): automation.validate_automation(single=True),

        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
)

LD6001BBaseSchema = cv.Schema(
    {
        cv.GenerateID(CONF_LD6001B_ID): cv.use_id(LD6001BComponent),
    },
)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "ld6001b",
    require_tx=True,
    require_rx=True,
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    cg.add(var.set_throttle(config[CONF_THROTTLE]))

    if CONF_ON_UPDATE in config:
        await automation.build_automation(
            var.get_update_trigger(),
            [(People_t_const_ref, "targets")],
            config[CONF_ON_UPDATE],
        )

    if CONF_RESET_PIN in config:
        reset_pin = await cg.gpio_pin_expression(config[CONF_RESET_PIN])
        print(f"Reset pin: {reset_pin}")
        cg.add(var.set_reset_pin(reset_pin))