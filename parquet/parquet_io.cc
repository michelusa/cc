#include <iostream>
#include "arrow/io/file.h"
#include "arrow/buffer_builder.h"
#include "parquet/stream_reader.h"
#include "parquet/stream_writer.h"


#include "arrow/api.h"
#include "parquet/arrow/reader.h"
#include "parquet/arrow/writer.h"

//
// stream_in_parquet
// this is called by main to display
// the content of a parquet file, which follows the expected schema
void stream_in_parquet ( const std::string& parquet_filename )
{
	std::shared_ptr<arrow::io::ReadableFile> infile;

	PARQUET_ASSIGN_OR_THROW(
			infile,
			arrow::io::ReadableFile::Open(parquet_filename));
	parquet::StreamReader os{parquet::ParquetFileReader::Open(infile)};

	std::string name;
	int32_t age;
	std::string city;

	while ( !os.eof() )
	{
		os >> name >> age >> city >> parquet::EndRow;
		std::cout << "hello " << name << " from " << city << std::endl;
	}
}

std::shared_ptr<parquet::schema::GroupNode> get_schema()
{
	parquet::schema::NodeVector fields;
	fields.push_back(parquet::schema::PrimitiveNode::Make(
				"string_field",
				parquet::Repetition::OPTIONAL,
				parquet::Type::BYTE_ARRAY,
				parquet::ConvertedType::UTF8));

	fields.push_back(parquet::schema::PrimitiveNode::Make(
				"int32_field", parquet::Repetition::REQUIRED, parquet::Type::INT32,
				parquet::ConvertedType::INT_32));

	fields.push_back(parquet::schema::PrimitiveNode::Make(
				"string_field", parquet::Repetition::OPTIONAL, parquet::Type::BYTE_ARRAY,
				parquet::ConvertedType::UTF8));

	return std::static_pointer_cast<parquet::schema::GroupNode>(
			parquet::schema::GroupNode::Make("schema", parquet::Repetition::REQUIRED, fields));
}

//
void stream_out_parquet (const std::string& parquet_filename )
{
	std::shared_ptr<arrow::io::FileOutputStream> outfile;

	PARQUET_ASSIGN_OR_THROW(
			outfile,
			arrow::io::FileOutputStream::Open(parquet_filename));

	const std::vector<std::shared_ptr<arrow::Field>> schema_vector = {
		arrow::field("name", arrow::utf8()),
		arrow::field("age", arrow::int32()),
		arrow::field("city", arrow::utf8())
	};

	auto schema = std::make_shared<arrow::Schema>(schema_vector);

	parquet::WriterProperties::Builder builder;

	parquet::StreamWriter os {
		parquet::ParquetFileWriter::Open(outfile,
				get_schema(),
				builder.build())
	};

	os << "marco" << 32 << "Tokyo" << parquet::EndRow;
	os << "polo" << 45 << "SanFran" << parquet::EndRow;
}

std::shared_ptr<arrow::Table> generate_table() {
  arrow::StringBuilder strbuilder;
  PARQUET_THROW_NOT_OK(strbuilder.Append("Peter"));
  PARQUET_THROW_NOT_OK(strbuilder.Append("Paul"));
  std::shared_ptr<arrow::Array> strarray;
  PARQUET_THROW_NOT_OK(strbuilder.Finish(&strarray));

  arrow::Int32Builder i32builder;
  PARQUET_THROW_NOT_OK(i32builder.AppendValues({1, 2}));
  std::shared_ptr<arrow::Array> i32array;
  PARQUET_THROW_NOT_OK(i32builder.Finish(&i32array));

  arrow::StringBuilder citybuilder;
  PARQUET_THROW_NOT_OK(citybuilder.Append("Montreal"));
  PARQUET_THROW_NOT_OK(citybuilder.Append("Toronto"));
  std::shared_ptr<arrow::Array> cityarray;
  PARQUET_THROW_NOT_OK(citybuilder.Finish(&cityarray));

  std::shared_ptr<arrow::Schema> schema = arrow::schema(
      {
      arrow::field("str", arrow::utf8()),
      arrow::field("int", arrow::int32()),
      arrow::field("str", arrow::utf8())
      });

  return arrow::Table::Make(schema, {strarray, i32array, cityarray});
}



void write_parquet_file() {
const auto table = generate_table();
  std::shared_ptr<arrow::io::FileOutputStream> outfile;
  PARQUET_ASSIGN_OR_THROW(
      outfile,
      arrow::io::FileOutputStream::Open("block.parquet"));
  PARQUET_THROW_NOT_OK(
      parquet::arrow::WriteTable(*table, arrow::default_memory_pool(), outfile, 3));
}


auto main() -> int
{
	stream_out_parquet("bye.parquet");
	stream_in_parquet("bye.parquet");
	write_parquet_file();

	stream_in_parquet("block.parquet");
}
