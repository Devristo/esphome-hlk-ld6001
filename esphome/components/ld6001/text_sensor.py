import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import (
    CONF_VERSION,
    ENTITY_CATEGORY_DIAGNOSTIC,
    ICON_CHIP,
)

from . import CONF_LD6001_ID, LD6001Component

DEPENDENCIES = ["ld6001"]

MAX_TARGETS = 10

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_LD6001_ID): cv.use_id(LD6001Component),
        cv.Optional(CONF_VERSION): text_sensor.text_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            icon=ICON_CHIP,
        ),
    }
)


async def to_code(config):
    ld6001_component = await cg.get_variable(config[CONF_LD6001_ID])
    if version_config := config.get(CONF_VERSION):
        sens = await text_sensor.new_text_sensor(version_config)
        cg.add(ld6001_component.set_version_text_sensor(sens))
