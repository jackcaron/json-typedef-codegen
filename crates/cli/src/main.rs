mod cli;
mod root_name;

use anyhow::{format_err, Context, Result};
use clap::Parser;
use cli::Cli;
use jtd::{Schema, SerdeSchema};
use serde::Serialize;
use std::collections::BTreeMap;
use std::convert::TryInto;
use std::fs::File;
use std::io::Read;
use std::path::Path;

fn main() -> Result<()> {
    let cli = Cli::parse();

    let mut log: Box<dyn Log> = match cli.log_format.as_str() {
        "pretty" => Box::new(PrettyLog()),
        "minimal" => Box::new(MinimalLog()),
        "json" => Box::new(JsonLog(BTreeMap::new())),
        _ => unreachable!(),
    };

    let input = &cli.schema;

    // Determine the desired root name to pass to jtd_codegen. If the user has
    // supplied root-name, we'll use that. Otherwise, we'll infer a desired root
    // name from the name of the input file.
    let root_name =
        root_name::root_name_from_input_name(cli.root_name.as_deref().unwrap_or(input)).to_owned();

    let schema: Schema = {
        // Open, parse, and validate the input schema.
        let input_reader: Box<dyn Read> = match input.as_str() {
            "-" => Box::new(std::io::stdin()),
            _ => Box::new(File::open(input).with_context(|| "Failed to open input file")?),
        };
        let serde_schema: SerdeSchema = serde_json::from_reader(input_reader)
            .with_context(|| "Failed to parse input as JSON")?;
        serde_schema
            .try_into()
            .map_err(|err| format_err!("{:?}", err))
            .with_context(|| "Failed to validate input schema")?
    };

    // Generate code for all enabled targets.

    if let Some(out_dir) = &cli.csharp_system_text_out {
        log.start("C# + System.Text.Json", out_dir);

        let namespace = cli.csharp_system_text_namespace.as_ref().unwrap().clone();
        let target = jtd_codegen_target_csharp_system_text::Target::new(namespace);

        let codegen_info = jtd_codegen::codegen(&target, &root_name, &schema, &Path::new(out_dir))
            .with_context(|| "Failed to generate C# + System.Text.Json code")?;

        log.finish("C# + System.Text.Json", &codegen_info);
    }

    if let Some(out_dir) = &cli.go_out {
        log.start("Go", out_dir);

        let package = cli.go_package.as_ref().unwrap().clone();
        let target = jtd_codegen_target_go::Target::new(package);

        let codegen_info = jtd_codegen::codegen(&target, &root_name, &schema, &Path::new(out_dir))
            .with_context(|| "Failed to generate Go code")?;

        log.finish("Go", &codegen_info);
    }

    if let Some(out_dir) = &cli.java_jackson_out {
        log.start("Java + Jackson", out_dir);

        let package = cli.java_jackson_package.as_ref().unwrap().clone();
        let target = jtd_codegen_target_java_jackson::Target::new(package);

        let codegen_info = jtd_codegen::codegen(&target, &root_name, &schema, &Path::new(out_dir))
            .with_context(|| "Failed to generate Java + Jackson code")?;

        log.finish("Java + Jackson", &codegen_info);
    }

    if let Some(out_dir) = &cli.python_out {
        log.start("Python", out_dir);

        let target = jtd_codegen_target_python::Target::new();
        let codegen_info = jtd_codegen::codegen(&target, &root_name, &schema, &Path::new(out_dir))
            .with_context(|| "Failed to generate Python code")?;

        log.finish("Python", &codegen_info);
    }

    if let Some(out_dir) = &cli.ruby_out {
        log.start("Ruby", out_dir);

        let module = cli.ruby_module.as_ref().unwrap().clone();
        let target = jtd_codegen_target_ruby::Target::new(module);

        let codegen_info = jtd_codegen::codegen(&target, &root_name, &schema, &Path::new(out_dir))
            .with_context(|| "Failed to generate Ruby code")?;

        log.finish("Ruby", &codegen_info);
    }

    if let Some(out_dir) = &cli.ruby_sig_out {
        log.start("Ruby Signatures", out_dir);

        let module = cli.ruby_sig_module.as_ref().unwrap().clone();
        let target = jtd_codegen_target_ruby_sig::Target::new(module);

        let codegen_info = jtd_codegen::codegen(&target, &root_name, &schema, &Path::new(out_dir))
            .with_context(|| "Failed to generate Ruby Signatures code")?;

        log.finish("Ruby Signatures", &codegen_info);
    }

    if let Some(out_dir) = &cli.rust_out {
        log.start("Rust", out_dir);

        let extra_derives = cli.rust_derive.as_deref().unwrap_or_default();
        let target = jtd_codegen_target_rust::Target::new(&extra_derives);
        let codegen_info = jtd_codegen::codegen(&target, &root_name, &schema, &Path::new(out_dir))
            .with_context(|| "Failed to generate Rust code")?;

        log.finish("Rust", &codegen_info);
    }

    if let Some(out_dir) = &cli.cpp_out {
        log.start("C++", out_dir);

        use jtd_codegen_target_cpp::props::CppProps;

        let cpp_props = CppProps::from_file(cli.cpp_props.as_deref())?;
        let target = jtd_codegen_target_cpp::Target::new(cpp_props, &root_name);
        let codegen_info = jtd_codegen::codegen(&target, &root_name, &schema, &Path::new(out_dir))
            .with_context(|| "Failed to generate C++ code")?;

        log.finish("C++", &codegen_info);
    }

    if let Some(out_dir) = &cli.typescript_out {
        log.start("TypeScript", out_dir);

        let target = jtd_codegen_target_typescript::Target::new();
        let codegen_info = jtd_codegen::codegen(&target, &root_name, &schema, &Path::new(out_dir))
            .with_context(|| "Failed to generate TypeScript code")?;

        log.finish("TypeScript", &codegen_info);
    }

    log.flush();
    Ok(())
}

trait Log {
    fn start(&mut self, target: &str, out_dir: &str);
    fn finish(&mut self, target: &str, info: &jtd_codegen::codegen::CodegenInfo);
    fn flush(&mut self);
}

struct PrettyLog();
impl Log for PrettyLog {
    fn start(&mut self, target: &str, out_dir: &str) {
        use colored::*;

        println!(
            "📝 Writing {} code to: {}",
            target.green().bold(),
            out_dir.bold()
        );
    }

    fn finish(&mut self, target: &str, info: &jtd_codegen::codegen::CodegenInfo) {
        use colored::*;

        println!("📦 Generated {} code.", target.green().bold());
        println!(
            "📦     Root schema converted into type: {}",
            info.root_name.bold()
        );
        for (definition_name, type_name) in &info.definition_names {
            println!(
                "📦     Definition {} converted into type: {}",
                format!("{:?}", definition_name).bold(),
                type_name.bold()
            );
        }
    }

    fn flush(&mut self) {}
}

struct MinimalLog();
impl Log for MinimalLog {
    fn start(&mut self, target: &str, out_dir: &str) {
        eprintln!("{}: writing to: {}", target, out_dir);
    }

    fn finish(&mut self, target: &str, info: &jtd_codegen::codegen::CodegenInfo) {
        println!("{}: root: {}", target, &info.root_name);
        for (definition_name, type_name) in &info.definition_names {
            println!("{}: definition: {}: {}", target, definition_name, type_name);
        }
    }

    fn flush(&mut self) {}
}

struct JsonLog(BTreeMap<String, TargetEntry>);

#[derive(Serialize)]
struct TargetEntry {
    out_dir: String,
    root_name: String,
    definition_names: BTreeMap<String, String>,
}

impl Log for JsonLog {
    fn start(&mut self, target: &str, out_dir: &str) {
        self.0.insert(
            target.to_owned(),
            TargetEntry {
                out_dir: out_dir.to_owned(),
                root_name: "".to_owned(),
                definition_names: BTreeMap::new(),
            },
        );
    }

    fn finish(&mut self, target: &str, info: &jtd_codegen::codegen::CodegenInfo) {
        let entry = self.0.get_mut(target).unwrap();

        entry.root_name = info.root_name.clone();
        entry.definition_names = info.definition_names.clone();
    }

    fn flush(&mut self) {
        println!("{}", serde_json::to_string(&self.0).unwrap());
    }
}
