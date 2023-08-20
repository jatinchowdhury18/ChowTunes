cmrc_add_resource_library(gui-resources
    ALIAS chow_tunes::gui-resources
    NAMESPACE gui
    icon.png
)
set_target_properties(gui-resources PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
