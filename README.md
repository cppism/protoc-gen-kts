## Usage

[Demo project](https://github.com/cppism/protoc-gen-kts-demo)

### libs.versions.toml
```toml
[plugins]
kotlin-serialization = { id = "org.jetbrains.kotlin.plugin.serialization", version.ref = "kotlin" }
google-protobuf = { id = "com.google.protobuf", version.ref = "google-protobuf-plugin" }

[libraries]
kotlinx-serialization-protobuf = { group = "org.jetbrains.kotlinx", name = "kotlinx-serialization-protobuf", version.ref = "kotlinx-serialization" }
protobuf-protoc = { group = "com.google.protobuf", name = "protoc", version.ref = "google-protobuf" }
protobuf-gen-kts = { group = "io.github.cppism", name = "protoc-gen-kts", version.ref = "google-protobuf" }
```

### build.gradle.kts
```gradle
plugins {
    alias(libs.plugins.kotlin.serialization)
    alias(libs.plugins.google.protobuf)
}

protobuf {
    protoc {
        artifact = libs.protobuf.protoc.get().toString()
    }
    plugins {
        create("kts").artifact = libs.protobuf.gen.kts.get().toString()
    }
    generateProtoTasks {
        ofSourceSet("main").forEach { task ->
            task.builtins.clear()
            task.plugins.create("kts")
        }
    }
}
```