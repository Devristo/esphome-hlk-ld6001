import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_THROTTLE
from esphome import automation

DEPENDENCIES = ["uart"]
MULTI_CONF = True

ld6001_ns = cg.esphome_ns.namespace("ld6001")
LD6001Component = ld6001_ns.class_("LD6001Component", cg.PollingComponent, uart.UARTDevice)

CONF_LD6001_ID = "ld6001_id"
CONF_ON_TARGET_ENTER = "on_target_enter"
CONF_ON_TARGET_LEFT = "on_target_left"
CONF_ON_UPDATE = "on_update"

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(LD6001Component),
            cv.Optional(CONF_THROTTLE, default="1000ms"): cv.All(
                cv.positive_time_period_milliseconds,
                cv.Range(min=cv.TimePeriod(milliseconds=1)),
            ),
            cv.Optional(CONF_ON_TARGET_ENTER): automation.validate_automation(single=True),
            cv.Optional(CONF_ON_TARGET_LEFT): automation.validate_automation(single=True),
            cv.Optional(CONF_ON_UPDATE): automation.validate_automation(single=True),
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.polling_component_schema("500ms"))
)

LD6001BaseSchema = cv.Schema(
    {
        cv.GenerateID(CONF_LD6001_ID): cv.use_id(LD6001Component),
    },
)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "ld6001",
    require_tx=True,
    require_rx=True,
    parity="EVEN",
    stop_bits=1,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    cg.add(var.set_throttle(config[CONF_THROTTLE]))

    if CONF_ON_TARGET_ENTER in config:
        await automation.build_automation(
            var.get_target_enter_trigger(),
            [(cg.uint8, "target_id")],
            config[CONF_ON_TARGET_ENTER],
        )

    if CONF_ON_TARGET_LEFT in config:
        await automation.build_automation(
            var.get_target_left_trigger(),
            [(cg.uint8, "target_id"), (cg.uint32, "dwell_time")],
            config[CONF_ON_TARGET_LEFT],
        )

    if CONF_ON_UPDATE in config:
        await automation.build_automation(
            var.get_update_trigger(),
            [(People_t_const_ref, "targets")],
            config[CONF_ON_UPDATE],
        )
