import unreal

blueprint_path = "/Game/Controller/BP_KineticPlayerControlle"
parent_path = "/Script/FlailPong.KineticPlayerControllerBase"

blueprint = unreal.load_asset(blueprint_path)
parent_class = unreal.load_class(None, parent_path)
if not blueprint:
    raise RuntimeError("Could not load " + blueprint_path)
if not parent_class:
    raise RuntimeError("Could not load " + parent_path)

result = unreal.BlueprintEditorLibrary.reparent_blueprint(blueprint, parent_class)
print("Reparent result: " + str(result))
unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
unreal.EditorAssetLibrary.save_loaded_asset(blueprint)
print("Saved controller Blueprint")
