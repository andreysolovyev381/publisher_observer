### Publisher Observer pattern. 

* Arbitrary Event type, Notification (Value) type.
* Differs from Gang of 4 by using "push" model for data once it appears in Publisher state.
* Header only, copy-paste include into your project.
* Depends on Boost Circular Buffer. I was too lazy to separate that dependency, included entire Boost. For performance purposes such a buffer should be changed for something with less latency (very much achievable).
* See tests for tests and usage examples.

---

Distributed under MIT License, absolutely no guarantees are given to whomever whatsoever.
