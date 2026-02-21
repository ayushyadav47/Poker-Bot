if(NOT TARGET ImGui-SFML::ImGui-SFML)
    message(STATUS "ImGui-SFML target not found, assuming manual setup or different name.")
    # Fallback or manual setup could go here if needed, but for now we rely on the root fetch.
endif()

add_executable(gui_demo main.cpp)

target_link_libraries(gui_demo PRIVATE
    ImGui-SFML::ImGui-SFML
    sfml-graphics
    sfml-window
    sfml-system
    poker_engine
)

# If you want to link the PokerEngine logic later:
# target_link_libraries(gui_demo PRIVATE poker_engine)