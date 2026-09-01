# FloppyPong

Unreal Engine project repository.

## Git setup

This repository is configured for Unreal Engine development with:

- `.gitignore` rules for generated Unreal, IDE, build, packaged, and diagnostic files.
- `.gitattributes` rules for line ending normalization and Git LFS tracking of Unreal binary assets.

Before cloning or adding large Unreal assets on another machine, install Git LFS:

```powershell
git lfs install
```

Commit the `.uproject`, `Config/`, `Content/`, `Source/`, and project-specific `Plugins/` directories. Do not commit generated `Binaries/`, `DerivedDataCache/`, `Intermediate/`, or `Saved/` directories.
