# Binding specification: Java and TypeScript

Step 2 of S-16, `specs/2026-10-18-language-system-design.md`. The two languages that already have
implementations, written down so a second implementation of either is forced to match, and so the
per-language documents for Rust, Python and C have a worked example to follow.

Written 2026-10-18. Every signature below was read from the source that day. Where a subagent
inventory and the source disagreed, the source won and the disagreement is noted.

**Approved 2026-10-20, with section 10.** Paths under `specs/` and `plans/`, `ECOSYSTEM.md`,
`RULES.md` and task ids such as `S-16` name the records of the programme that produced this
document, kept in `spisor/docs`; they say where a ruling's grounds are, and nothing here depends
on reading them.

**Section 9 was added 2026-10-20**, when the four gaps of
`plans/2026-10-19-four-gaps-into-the-engine.md` were built into `:engine`. Sections 1 to 8
describe what exists; section 9 describes surface that now exists in Java and PRESCRIBES its
TypeScript rendering, in the same sense the scheduler row of section 3 does.

---

## 1. What a binding specification is, and is not

**It pins the SURFACE in one language**: names, shapes, and how each layer-1 invariant is rendered.
It exists so two implementations of the same language are interchangeable, which a conformance suite
cannot deliver on its own: a plugin compiles against types and names, not against behaviour.

**It is not the model**, which is language-neutral and lives in `weivel/model`. It is not a tutorial, and
it does not describe the engine's internals, which are deliberately each implementation's own.

## 2. Layer 1: the invariants, which are not per language

**They live in `weivel/model`'s `INVARIANTS.md` since 2026-10-20**, beside the schema, and are not
repeated here: two copies of a property list are two lists that will disagree. The sections below
cite them by number, and the numbers are that file's. Each carries the reason it exists, which a binding
author needs before rendering it.

## 3. The concept mapping, verified

Java is `io.github.intisy.ai.api` in bayonet's `contract` module. TypeScript is
`@intisy/bayonet`'s generated `api.d.ts`.

| concept | Java | TypeScript | invariant |
| --- | --- | --- | --- |
| typed key | `CapabilityType<T>`, `ServiceType<T>`, `TopicType<T>` | same three names, `{ readonly __phantom?: T; readonly id: string }` | |
| what a plugin implements | `Plugin`: `void activate(PluginContext)`, `void deactivate()`, `void install(PluginContext)`, `void repair(PluginContext)` | `Plugin` with `activate`/`deactivate` required, `install?`/`repair?` optional, each `void \| Promise<void>` | |
| what a plugin is handed | `PluginContext` | `PluginContext` | |
| register a capability | `PluginContext.provide(CapabilityType<T>, T)` | `provide<T>(type, implementation): void` | |
| service registry | `Services`: `<T> T get(ServiceType<T>)`, `<T> CompletionStage<T> want(ServiceType<T>)`, `<T> CompletionStage<T> want(ServiceType<T>, WantOptions)`, `<T> Runnable watch(ServiceType<T>, BiConsumer<T,String>)`, `<T> Runnable register(ServiceType<T>, T)`, `List<String> ids()` | `Services` with the same six: `get<T>(type): T \| undefined`, `want<T>(type): Promise<T>`, `want<T>(type, options: WantOptions): Promise<T>`, `watch<T>(type, listener: (a: T, b: string) => void): () => void`, `register<T>(type, implementation): () => void`, `ids(): string[]` | 8, 10, 11 |
| events | `Events`: `<T> void publish(TopicType<T>, T)`, `<T> Runnable subscribe(TopicType<T>, Consumer<T>)` | `publish<T>(topic, payload): void`, `subscribe<T>(topic, listener): () => void` | 10 |
| absent value | `null` return from `get` | `T \| undefined` | 1 |
| deferred value | `CompletionStage<T>` | `Promise<T>` | 8 |
| disposer | `Runnable` | `() => void` | 10 |
| timeout | `WantOptions` | `WantOptions { timeoutMs?: number }` | 8 |
| refusal | `PluginException`, RETURNED, with `null` meaning acceptable | `PluginError { name: "PluginError"; pluginId; detail; fix; message }`, recognised by the `name` marker rather than by type identity | 1 |
| host identity | `HostDescriptor`: `api`, `app`, `surfaces` | same three fields | 2 |
| api floor | `Api.API_VERSION = 2`, and `int api` per manifest | `API_VERSION: number`, the package's only runtime export | 2 |
| **scheduling** | `Scheduler.schedule(Runnable task, long delayMillis)` returning `Cancellable`, with `Scheduler.NEVER`; **engine-internal today, moving onto the contract** | `Scheduler { schedule(task: () => void, delayMillis: number): () => void }` on the contract, with a `NEVER` scheduler; cancelling is the returned disposer, by naming rule 2 below | 8 |

**The scheduler row is the one line of this table that prescribes rather than describes**, ruled
2026-10-18 (section 8, question 1). Every deadline in the system comes from the host-supplied
scheduler: activation, deactivation, `want` timeouts and watch delivery. Java moves `Scheduler` out
of the engine and onto the contract; TypeScript gains it, in the shape above, fixed 2026-10-20 for
the native implementation. Until both land, the two existing bindings disagree here and section 6
records why that is a defect rather than a gap.

**The refusal type is `PluginError`, corrected 2026-10-20.** This row used to name
`PluginErrorShape`, which exists only in the TeaVM bundle's `engine.d.ts` and which section 5
excludes as bridge vocabulary; the fields and the `name` marker are unchanged.

**Four further concepts entered the model on 2026-10-20** and are spelled in section 9 rather than
in this table, because each needs its rules beside its names: a requirement's policy, the edge
source of the ordering, a provider that answers under constraints, and the handle a withdrawn
service is revoked through.

**Three naming rules, which the two existing bindings already follow and which are now normative:**

1. **One concept, one name, in every language**, adjusted only for the language's casing convention.
   `Services` is `Services`, `PluginContext` is `PluginContext`. A binding does not get to improve a
   name.
2. **A disposer is the language's smallest callable**, returned by value. Java `Runnable`, TypeScript
   `() => void`, and in a language with destructors it is still a returned value, never implicit
   drop, because invariant 10 requires revocation to be explicit and observable.
3. **A deferred value is the language's own idiom**, not a ported type. `CompletionStage` and
   `Promise` are correct; a hand-rolled `Pending` in a third language would be wrong.

## 4. The finding that makes this tractable

**The Java contract contains no Java-only machinery.** Verified by reading all six `Services`
signatures, both `Events` signatures and all four `Plugin` signatures: the contract uses generics,
`Runnable` as a disposer, `CompletionStage` as the deferred type, nullable returns and plain data.
Nothing else.

`Pending`, `Scheduler` and `Scheduler.Cancellable` appear **nowhere in the contract**. They are
engine-internal, which is precisely where implementations are supposed to differ. So the surface a
plugin or host programs against was already language-neutral before anyone tried to specify it, and
the binding spec is a mapping rather than a redesign.

**The TypeScript binding already exists in machine-readable form**, and this is the ruling that
decides step 2 for TypeScript: `generated/api.d.ts` is **845 lines, 36 exported interfaces, zero
functions and zero constants**, emitted from the Java contract. The package's only runtime export is
`API_VERSION`. **That declaration file IS the normative TypeScript surface.** A native TypeScript
implementation is correct when it satisfies it, and a second implementation is interchangeable when
it does too.

(The inventory that produced this said 35 interfaces. It is 36. The source won.)

## 5. What is NOT part of the TypeScript binding

Everything on the `./engine` subpath. `HostSurface`, `ServiceRegistryShape`, `ContextSurface`,
`EventBusShape`, `PluginErrorShape`, `ActivationPlanShape` and the rest of the `*Shape` family are
the **TeaVM bridge's own vocabulary**: renamed, and with every payload erased to `unknown` because a
compiled bundle cannot carry generics across the boundary. They exist only because the
implementation is compiled Java, and they disappear with it.

The evidence that they are a bridge rather than a binding is one line,
`bayonet/src/host/plugin-host.ts:308`:

```ts
context = host.contextFor(manifest, options.runtimeFor(manifest)) as unknown as PluginContext;
```

That single `as unknown as` is the only join between the typed contract and the erased engine
surface. **A native TypeScript implementation removes that cast rather than reproducing it**, and
nothing in `api.d.ts` changes when it does.

## 6. Gaps and defects the inventory exposed

Recorded here because step 3 will meet all of them, and three are defects rather than gaps.

**Not a gap after all: `ManifestFacts` is absent from TypeScript, and that is now the correct
shape.** Ruled 2026-10-18 as implementation-private, so Java's exposure is the anomaly to narrow
rather than TypeScript's absence to fill. What still binds either way is **invariant 4**: whatever an
implementation builds internally must hand the host's own object back by identity rather than a
reconstruction.

**A real defect, and now resolved in principle: timing lives in two places.** `Scheduler` is absent
from TypeScript and not injectable; the bundle's own scheduler uses `setTimeout` internally, while
the TypeScript driver keeps its own `setTimeout` deadlines for activation, deactivation and
capability calls. **Two independent clocks govern one plugin lifecycle, and neither knows about the
other.** Ruled 2026-10-18: one clock, host-supplied, on the binding surface, which is the scheduler
row of section 3. The work is on both implementations, not only the new one.

**Defect: one hand-written mirror has already drifted, with the drift live in the published API.**
`basekit/src/activity.types.ts` declares `ActivitySpec` (:78), `ActivityRecord` (:104) and
`ActivityQuery` (:179) under the same names as `src/generated/contracts.ts` (:873, :612, :981), and
the shapes differ materially: the hand-written `ActivitySpec` carries `actor`, `subject`, `target`,
`cause`, `outcome`, `durationMs` and `changes` where the generated one has `topic`, `action`,
`impact?`, `subject?` and `details?`. `src/index.ts:42` explicitly exports the hand-written three
while `:31` type-exports the generated ones, so the explicit export wins and every consumer gets the
hand-written shapes, while the generated `ActivityService` contract is defined in terms of the
generated ones. Method bivariance is why TypeScript allows it.
This is the failure mode `basekit/src/auth/provider-plugin.ts` already carries a comment about, from
a previous occurrence. It needs its own queue entry; it is not a binding question.

**Defect, minor: a stale pin.** `stub-plugin/package.json` requires `@intisy-ai/basekit` at `^5.0.0`
while basekit is at 6.0.0. Reference plugins are meant to be the thing kept green against every
gate.

## 7. What step 3 has to satisfy

A native TypeScript implementation is done when:

1. It satisfies `generated/api.d.ts` unchanged, and the cast at `plugin-host.ts:308` is deleted
   rather than moved.
2. It passes the conformance suite for all eleven invariants of section 2, with each test
   demonstrated to fail against the invariant's absence before it counts.
3. It owns timing once, and whatever that decides is written back into section 3 as part of the
   surface or explicitly excluded from it.
4. The TeaVM path still works until 1 and 2 hold, per S-16's migration rule that nothing is deleted
   before its replacement is proven.

## 8. Open questions

All four were answered on 2026-10-18. They are kept rather than deleted because each answer is only
legible beside the question it settles.

1. ~~Is a scheduler part of the binding surface?~~ **YES, and it is one clock.** The binding exposes
   a scheduler the host supplies, and every deadline comes from it: activation and deactivation
   timeouts, `want` timeouts, and watch delivery. This is the one answer that changes both existing
   implementations rather than just describing them: Java moves `Scheduler` from engine-internal
   into the contract, and TypeScript gains it where it has nothing today. **It also ends the
   two-clock split** of section 6, which was the real defect: the driver's `setTimeout` deadlines
   and the bundle's internal scheduler currently govern one lifecycle without knowing about each
   other. Section 3's mapping gains a row, and timing becomes testable and deterministic, which the
   conformance suite needs for invariant 8.
2. ~~Is `ManifestFacts` part of the binding?~~ **NO, it is implementation-private.** Invariant 4
   already pins what it must do, carry the host's manifest object through untouched and hand back
   the same object, and nothing outside the engine reads the facts type. TypeScript proves a binding
   works without it. **So Java's exposure is the anomaly to narrow**, not TypeScript's absence to
   fill, which is the opposite of what the gap in section 6 implied.
3. ~~Does the plugin entry shape belong here?~~ **YES, specified per language.** Each binding states
   exactly how a host finds a plugin's entry point in that language. Today's duck-typed fallback,
   where a Java-authored plugin is reached through named exports because TeaVM cannot emit an object
   as a default, is a bridge artifact: unspecified, it would be copied forward as though it were the
   design. A Rust plugin needs the real answer.
4. ~~Where do the 18 product-vocabulary types in `contract` go?~~ **Declared product-scoped in
   place, not moved**, ruled the same day, so an implementer knows the 28 model types immediately
   while the 21 npm and 32 Gradle consumers keep working. Of the 46: twelve `App*` types plus
   `HomeDescriptor`, `RepoMeta`, `MarketplaceCategory`, `MarketplaceMatch`, `ManifestMarketplace`
   and `ManifestPublish` are the product domain. Same mechanism as F-26, which was ruled the same way
   on the same day: `MODULE_SCOPES` plus a `scope-check` gate. `ECOSYSTEM.md` rev 93 (c) and (d).

---

## 9. The four gaps' surface, added 2026-10-20

**Why this section is here rather than in the conformance kit.** The kit makes two implementations
DECIDE the same things. It does not make two implementations of ONE language interchangeable,
because a plugin compiles against type names and error shapes rather than against behaviour. All
four gaps added surface, so all four have to be spelled.

The language-neutral model comes first in each case, because a Rust or C implementer reads that and
not the Java.

### 9.1 A requirement says what an absence costs

**Model.** A requirement names a service and carries three further facts: whether its absence stops
the consumer (`optional`), whether an offer that never satisfies its constraints is acceptable once
the budget is spent (`degradable`), and what it needs of the service (`constraints`, a map of
constraint name to number). **`optional` and `degradable` are different axes**: a requirement that
is degradable and not optional is still refused when nothing offers the service at all.

**Layer 1, so not per language:** `services.consumes` takes a bare string OR an object with `id`,
`optional`, `degradable` and `constraints`, and `services.provides` takes a bare string OR an object
with `id` and `attributes`. **A bare string means `{optional: true, degradable: false}`**, which is
what every manifest written before the object form already meant. `schema/plugin.schema.json` is the
source of both.

| | Java | TypeScript |
| --- | --- | --- |
| the manifest shape | `ManifestServices.consumes(): List<Object>`, emitted as the union | `consumes?: (string \| ManifestRequirement)[]` |
| one requirement | `ManifestRequirement { String id(); Boolean optional(); Boolean degradable(); Map<String, Double> constraints(); }` | `ManifestRequirement { id: string; optional?: boolean; degradable?: boolean; constraints?: Record<string, number> }` |
| one offer | `ManifestOffer { String id(); Map<String, Double> attributes(); }` | `ManifestOffer { id: string; attributes?: Record<string, number> }` |
| the parsed form | `ServiceRequirement.of(Object)`, `ServiceOffer.of(Object)` | a reader of the same two shapes; the engine never sees the union again once it has parsed |

### 9.2 Activation refuses, and the edge source is a parameter

**Model.** Ordering takes a set of units and ONE edge source. A unit declares what it offers and
what it needs, each marked as a service or a capability; declarations of the other kind are ignored
entirely, for ordering and for refusal alike. The plan it answers with carries the order, the
cycles, and one refusal per unit whose mandatory need nothing in the set meets. **A refusal
propagates**: a consumer whose only provider was itself refused is refused too. **A cycle is
reported with its members and never thrown**, and the units in it are left out of the order.

| | Java | TypeScript |
| --- | --- | --- |
| the edge source | `DeclarationKind { SERVICE, CAPABILITY }` | `type DeclarationKind = "service" \| "capability"` |
| a unit | `ActivationUnit(String id, List<Offer> provides, List<Need> requires)` | `{ id: string; provides: Offer[]; requires: Need[] }` |
| an offer | `ActivationUnit.Offer(DeclarationKind, String id)` | `{ kind: DeclarationKind; id: string }` |
| a need | `ActivationUnit.Need(DeclarationKind, String id, boolean optional, boolean degradable)` | `{ kind: DeclarationKind; id: string; optional: boolean; degradable: boolean }` |
| the call | `Activation.order(List<ActivationUnit>, DeclarationKind)` | `order(units, edgeSource)` |
| the answer | `ActivationPlan`: `getOrder()`, `getCycles()`, `getRefusals()` | `{ order: string[]; cycles: string[][]; refusals: PluginError[] }` |

A refusal is a refusal VALUE, by invariant 1, so Java returns `PluginException` instances rather
than throwing the first one: a host quarantines the units it cannot start and starts the rest.

### 9.3 A registration can answer for itself

**Model.** A registration may be a bare instance or a provider. A provider answers two questions:
what it offers right now, and whether it will serve a consumer asking under given constraints. A
consumer's constrained wait re-asks on a schedule until a budget is spent, and then either degrades
with a report or refuses.

**Three rules a binding must not render away:**

1. **The overlay is per key.** Live attributes overlay the declaration's advertised claim key by
   key, and an EMPTY live map means "nothing live to add", which leaves the whole advertised claim
   standing rather than erasing it.
2. **The constraint vocabulary belongs to the host**, never to the engine: which attribute a
   constraint is compared against, and in which direction. A constraint no declared vocabulary
   knows is NOT satisfied, and the engine says so through its diagnostics. An engine that shipped
   `minReplicas` would be one product's engine.
3. **Degradation is never silent.** The degradable path asks once more with the consumer's own
   constraints before asking unconstrained, and reports what it accepted before handing it over.

| | Java | TypeScript |
| --- | --- | --- |
| a provider | `ServiceProvider { Map<String, Double> attributes(); Object serviceFor(Map<String, Double> constraints); }` | `{ attributes(): Record<string, number>; serviceFor(constraints: Record<string, number>): unknown \| undefined }` |
| a bare instance | `ServiceProviders.constant(Object)` | the same wrapper, applied by the registry |
| registering | `register(String id, ServiceProvider, Map<String, Double> advertised)` | `register(id, provider, advertised)` |
| the vocabulary | `ConstraintVocabulary.of(List<Entry>)`, `Entry(constraintName, attributeName, Bound.LOWER \| Bound.UPPER)` | `{ constraintName: string; attributeName: string; bound: "lower" \| "upper" }[]` |
| the budget | `Budget.of(long budgetMillis, long pollMillis)` | `{ budgetMillis: number; pollMillis: number }` |
| the wait | `want(ServiceRequirement, Budget): Pending<Object>` | `want(requirement, budget): Promise<unknown>` |

**The budget is spent in scheduled ticks, never slept**, by invariant 8: every deadline comes from
the host's scheduler, which is also what lets a 30 second budget cost a test nothing.

### 9.4 A withdrawn service tells the consumer still holding it

**Model.** What a consumer receives from a lookup is a HANDLE over the instance. When the
registration is withdrawn, a call through a handle fails with a revoked error naming the service
rather than reaching a dead object. How the wrapping is done is platform-specific, so it is a seam
the host supplies; the engine owns only the rule.

**The fallback must report, not pretend.** A host that supplies no wrapping gets the instance as it
is, and the engine says so at the moment a withdrawal it cannot enforce actually happens. A service
that cannot be wrapped at all, such as a JVM class implementing no interface, is reported the same
way rather than quietly handed over.

| | Java | TypeScript |
| --- | --- | --- |
| the seam | `Handles { Handle around(String serviceId, Object service); }` | `{ around(serviceId: string, service: unknown): Handle }` |
| a handle | `Handles.Handle { Object get(); void revoke(); }` | `{ get(): unknown; revoke(): void }` |
| the default | `Handles.UNWRAPPED` | the same, reporting on revocation |
| the platform backing | `io.github.intisy.ai.seam.jvm.ReflectiveHandles`, a dynamic proxy over the interfaces the service implements | a backing over the language's own `Proxy`, which needs no reflection |
| the failure | `ServiceRevoked(String serviceId)`, a `RuntimeException` | a thrown error carrying `serviceId`, recognised by a `name` marker as `PluginError` is |

**`toString` answers on a revoked handle rather than failing.** A logger stringifying a dead handle
inside a catch block would otherwise throw from within error handling and replace the real failure.
This is a rule rather than a detail: the kernel learned it by measurement in September 2026.

### 9.5 What this section deliberately does not pin

- **The constraint names themselves.** `minReplicas` and `maxLatencyMs` are one product's words,
  declared by the host, and an engine that knew them would be that product's engine.
- **Whether the ordering is Tarjan's.** Cycle DETECTION is pinned; the algorithm is not.
- **How a handle is represented.** Only that a call through a revoked one fails, naming the service.

---

## 10. Loading a C library, added 2026-10-20

S-29. The library tier of `ECOSYSTEM.md` rev 113 rests on one capability: an implementation loads
and calls a C library, so every library ever written works on it and the binding matrix is N rather
than N times M. This section makes that a requirement of every implementation and pins how the two
existing languages render it. Both renderings below were run on 2026-10-20, calling the C runtime's
`strlen` on Windows x64, not read from documentation. The TypeScript rendering was changed from
`node:ffi` to koffi on 2026-10-26 after the measurements in the second table.

### 10.1 What every implementation provides

**It is HOST surface, never plugin surface.** A plugin never names a library (rev 113 (a)), so
nothing here appears on `PluginContext`. The implementation loads the library and registers what it
adds as services, which plugins reach through `ctx.services` like any other. In Java it lives in the
seam backing, beside `ReflectiveHandles`, and `engine` stays dependency-free.

**What crosses the boundary is C-expressible, with a named owner for every allocation**, rev 113
(f)'s first condition, one notch harsher than RULES.md section 7's wire-protocol rule:

| crosses as | not as |
| --- | --- |
| fixed-width integers, `double`, `bool` | a language's own number tower or boxed types |
| an opaque handle the host never dereferences | a struct the host reads field by field |
| a byte span, pointer plus length; text is a UTF-8 span | a NUL-terminated string assumed safe to scan |
| a status value plus a library-owned message | an exception, which C cannot raise and a host cannot catch across the boundary |

**Memory a library returns is freed by that library's own release function**, named per allocation
in its declaration, and never by the host's allocator. A lifetime bug here crashes the host rather
than returning a refusal, which is why ownership is pinned and not left to each binding.

**The binary is acquired by code**, rev 113 (f)'s second condition: a library publishes one shared
object per platform from one reusable workflow, and the implementation resolves and fetches the one
for the platform it runs on. An operator installs nothing.

### 10.2 The renderings, measured

| | Java | TypeScript |
| --- | --- | --- |
| mechanism | **JNA**, `net.java.dev.jna:jna:5.19.1` | **koffi 2.16.3**, an npm package that ships prebuilt binaries |
| proven | `Native.load("msvcrt", Crt.class).strlen("weivel")` returned `6`, compiled `--release 8` | `koffi.load("msvcrt.dll").func("size_t strlen(const char *)")("weivel")` returned `6`, unflagged, on Node 26.3.1, on Electron 32.3.3's Node 20.18.1, and on Bun 1.4.3 |
| footprint | 1.95 MB jar, class version 52, dispatch libraries for 27 platforms inside it | one npm dependency carrying its own per-platform binaries |
| where | the seam backing, supplied once per host, never by a plugin (the plugin weight rule) | the native TypeScript implementation of S-16 step 3 |
| its condition | none beyond the dependency | none beyond the dependency, on every runtime measured |

**Why koffi and not `node:ffi`**, measured 2026-10-26 on Windows x64, calling `strlen` with `"weivel"`:

| runtime | `node:ffi` | koffi 2.16.3 |
| --- | --- | --- |
| Node 26.3.1 | `ERR_UNKNOWN_BUILTIN_MODULE` unflagged; works only with `--experimental-ffi` | `6`, unflagged |
| Electron 32.3.3 (Node 20.18.1) | absent | `6`, unflagged |
| Bun 1.4.3 (released 2026-10-10) | does not exist | `6`, unflagged, against `msvcrt.dll`, koffi installed with npm into a scratch directory |

The hosts this tier must serve include Electron and OpenCode, which runs on Bun, and `node:ffi` is
absent on both, so it cannot be the rendering. koffi runs on all three, so one TypeScript backend
serves every host measured and no second backend is needed for Bun.

**Why not the alternatives, each ruled out on a measured or stated rule rather than on taste:**

- **Panama's FFM API** needs JDK 22 and the Java implementation compiles at release 8, a rule that
  exists to keep a TeaVM 1.8 route open (`docs/apps/CLAUDE.md`, Java source level).
- **JNI** needs C glue per library, compiled per platform, which doubles the per-platform binaries
  that condition two exists to keep to one per library.
- **`node:ffi`** in TypeScript, the first choice here, is absent on Electron and Bun and unflagged
  only from a Node release newer than the one measured (see the table above).
- **The TeaVM bundle cannot load C at all.** It runs in a JavaScript engine with no FFI. That is
  no requirement on it, because it is the bridge S-16 retires; a host with no FFI (a browser)
  reaches a library only through rev 113 (e)'s WASM trigger.

### 10.3 The finding that decides how signatures are written

**`node:ffi` does not reject a misspelt signature key, it changes the meaning.** Measured: writing
`result` where it expects `return` gave a function returning `undefined`, and writing `parameters`
where it expects `arguments` gave a function declared with zero arguments that then refused the
call. JNA has the mirror hazard: its signature is a Java interface that is not checked against the
C header, so a wrong width truncates or corrupts silently.

**So a binding never writes a library signature by hand.** Each implementation generates its FFI
signatures and the typed service a plugin sees from one machine-readable declaration the library
publishes. That keeps the matrix N: a library writes its declaration once, and an implementation
writes one generator, not one wrapper per library. **The declaration's format is not decided here**
and is S-32.

### 10.4 What this section deliberately does not pin

- **The declaration format and where it lives**, which is S-32.
- **A conformance vector family.** S-29 defers it until a second implementation exists to measure,
  since one implementation passing its own vectors proves nothing about the requirement.
- **Callbacks from C into the host.** basekit's event bus and streams may need them; nothing measured
  here does, and both mechanisms have one (`Callback` in JNA, `koffi.register` in koffi), so
  the question is the ownership of a callback's lifetime, not whether it is possible.
