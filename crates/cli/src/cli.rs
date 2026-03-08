use clap::Parser;

#[derive(Parser)]
#[command(name = "jtd-codegen")]
#[command(version = env!("CARGO_PKG_VERSION"))]
pub struct Cli {
    #[arg(help = "Input schema file. To read schema from stdin, use \"-\".")]
    pub schema: String,

    #[arg(long, value_name = "name", required_if_eq("schema", "-"))]
    pub root_name: Option<String>,

    #[arg(long, value_name = "fmt", value_parser = ["pretty", "minimal", "json"], default_value = "pretty")]
    pub log_format: String,

    // C# + System.Text.Json
    #[arg(
        long = "csharp-system-text-out",
        value_name = "dir",
        requires = "csharp_system_text_namespace"
    )]
    pub csharp_system_text_out: Option<String>,

    #[arg(long = "csharp-system-text-namespace", value_name = "namespace")]
    pub csharp_system_text_namespace: Option<String>,

    // Go
    #[arg(long = "go-out", value_name = "dir", requires = "go_package")]
    pub go_out: Option<String>,

    #[arg(long = "go-package", value_name = "package")]
    pub go_package: Option<String>,

    // Java + Jackson
    #[arg(
        long = "java-jackson-out",
        value_name = "dir",
        requires = "java_jackson_package"
    )]
    pub java_jackson_out: Option<String>,

    #[arg(long = "java-jackson-package", value_name = "package")]
    pub java_jackson_package: Option<String>,

    // Python
    #[arg(long = "python-out", value_name = "dir")]
    pub python_out: Option<String>,

    // Ruby
    #[arg(long = "ruby-out", value_name = "dir", requires = "ruby_module")]
    pub ruby_out: Option<String>,

    #[arg(long = "ruby-module", value_name = "package")]
    pub ruby_module: Option<String>,

    // Ruby Signatures
    #[arg(
        long = "ruby-sig-out",
        value_name = "dir",
        requires = "ruby_sig_module"
    )]
    pub ruby_sig_out: Option<String>,

    #[arg(long = "ruby-sig-module", value_name = "package")]
    pub ruby_sig_module: Option<String>,

    // Rust
    #[arg(long = "rust-out", value_name = "dir")]
    pub rust_out: Option<String>,

    #[arg(long = "rust-derive", value_name = "derive", requires = "rust_out")]
    pub rust_derive: Option<String>,

    // C++
    #[arg(long = "cpp-out", value_name = "dir")]
    pub cpp_out: Option<String>,

    #[arg(long = "cpp-props", value_name = "dir", requires = "cpp_out")]
    pub cpp_props: Option<String>,

    // TypeScript
    #[arg(long = "typescript-out", value_name = "dir")]
    pub typescript_out: Option<String>,
}
