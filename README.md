# OpenPortal

Implementation of seamless portals in Unreal Engine 5.2

# Usage

* Enable "Global Clipping Plane" in project settings
* Disable MotionBlur in PostProcessVolume if you have one and it affects your portal actors
* For projectile-like actors (which are logically created by another actor) it is necessary to set Owner field for correct teleportation. For example imagine character which is shooting with weapon close to the portal and it can happen that spawned projectile (bullet) will appear behind the portal which prevents it from teleportation.
