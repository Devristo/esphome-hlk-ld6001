import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_DISTANCE,
    ENTITY_CATEGORY_CONFIG,
    ICON_TIMELAPSE,
    UNIT_CENTIMETER,
    UNIT_SECOND,
)

from .. import CONF_LD6001_ID, LD6001Component, ld6001_ns

CONF_PRESENCE_TIMEOUT = "presence_timeout"
CONF_X1 = "x1"
CONF_X2 = "x2"
CONF_Y1 = "y1"
CONF_Y2 = "y2"
ICON_ARROW_BOTTOM_RIGHT = "mdi:arrow-bottom-right"
ICON_ARROW_BOTTOM_RIGHT_BOLD_BOX_OUTLINE = "mdi:arrow-bottom-right-bold-box-outline"
ICON_ARROW_TOP_LEFT = "mdi:arrow-top-left"
ICON_ARROW_TOP_LEFT_BOLD_BOX_OUTLINE = "mdi:arrow-top-left-bold-box-outline"
MAX_ZONES = 4

PresenceTimeoutNumber = ld6001_ns.class_("PresenceTimeoutNumber", number.Number)
ZoneCoordinateNumber = ld6001_ns.class_("ZoneCoordinateNumber", number.Number)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_LD6001_ID): cv.use_id(LD6001Component),
        # cv.Required(CONF_PRESENCE_TIMEOUT): number.number_schema(
        #     PresenceTimeoutNumber,
        #     unit_of_measurement=UNIT_SECOND,
        #     entity_category=ENTITY_CATEGORY_CONFIG,
        #     icon=ICON_TIMELAPSE,
        # ),
    }
)

CONFIG_SCHEMA = CONFIG_SCHEMA.extend(
    {
        cv.Optional(f"zone_{n + 1}"): cv.Schema(
            {
                cv.Required(CONF_X1): number.number_schema(
                    ZoneCoordinateNumber,
                    device_class=DEVICE_CLASS_DISTANCE,
                    unit_of_measurement=UNIT_CENTIMETER,
                    entity_category=ENTITY_CATEGORY_CONFIG,
                    icon=ICON_ARROW_TOP_LEFT_BOLD_BOX_OUTLINE,
                ),
                cv.Required(CONF_Y1): number.number_schema(
                    ZoneCoordinateNumber,
                    device_class=DEVICE_CLASS_DISTANCE,
                    unit_of_measurement=UNIT_CENTIMETER,
                    entity_category=ENTITY_CATEGORY_CONFIG,
                    icon=ICON_ARROW_TOP_LEFT,
                ),
                cv.Required(CONF_X2): number.number_schema(
                    ZoneCoordinateNumber,
                    device_class=DEVICE_CLASS_DISTANCE,
                    unit_of_measurement=UNIT_CENTIMETER,
                    entity_category=ENTITY_CATEGORY_CONFIG,
                    icon=ICON_ARROW_BOTTOM_RIGHT_BOLD_BOX_OUTLINE,
                ),
                cv.Required(CONF_Y2): number.number_schema(
                    ZoneCoordinateNumber,
                    device_class=DEVICE_CLASS_DISTANCE,
                    unit_of_measurement=UNIT_CENTIMETER,
                    entity_category=ENTITY_CATEGORY_CONFIG,
                    icon=ICON_ARROW_BOTTOM_RIGHT,
                ),
            }
        )
        for n in range(MAX_ZONES)
    }
)


async def to_code(config):
    ld6001_component = await cg.get_variable(config[CONF_LD6001_ID])
    for zone_num in range(MAX_ZONES):
        if zone_conf := config.get(f"zone_{zone_num + 1}"):
            zone_x1_config = zone_conf.get(CONF_X1)
            x1 = cg.new_Pvariable(zone_x1_config[CONF_ID], zone_num)
            await number.register_number(
                x1, zone_x1_config, min_value=-1280, max_value=1270, step=1
            )
            await cg.register_parented(x1, config[CONF_LD6001_ID])

            zone_y1_config = zone_conf.get(CONF_Y1)
            y1 = cg.new_Pvariable(zone_y1_config[CONF_ID], zone_num)
            await number.register_number(
                y1, zone_y1_config, min_value=-1280, max_value=1270, step=1
            )
            await cg.register_parented(y1, config[CONF_LD6001_ID])

            zone_x2_config = zone_conf.get(CONF_X2)
            x2 = cg.new_Pvariable(zone_x2_config[CONF_ID], zone_num)
            await number.register_number(
                x2, zone_x2_config, min_value=-1280, max_value=1270, step=1
            )
            await cg.register_parented(x2, config[CONF_LD6001_ID])

            zone_y2_config = zone_conf.get(CONF_Y2)
            y2 = cg.new_Pvariable(zone_y2_config[CONF_ID], zone_num)
            await number.register_number(
                y2, zone_y2_config, min_value=-1280, max_value=1270, step=1
            )
            await cg.register_parented(y2, config[CONF_LD6001_ID])

            cg.add(ld6001_component.set_zone_numbers(zone_num, x1, y1, x2, y2))
