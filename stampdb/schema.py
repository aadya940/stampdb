from .point import Point

import json

class SchemaValidation:
    def __init__(self, schema: list = None, filename: str = None):
        if schema is None and filename is None:
            raise ValueError("Either schema or filename should be provided.")

        self.schema = schema
        self.filename = filename

        if filename is not None and schema is None:
            self._load_schema_from_file()
    
    def _load_schema_from_file(self):
        """Load schema from a JSON file."""
        try:
            with open(self.filename, 'r') as file:
                self.schema = json.load(file)
        except FileNotFoundError:
            raise FileNotFoundError(f"Schema file '{self.filename}' not found.")
        except json.JSONDecodeError as e:
            raise ValueError(f"Invalid JSON in schema file '{self.filename}': {e}")
        except Exception as e:
            raise RuntimeError(f"Error loading schema from '{self.filename}': {e}")

    def _save_schema_to_file(self):
        """Save the current schema to a JSON file."""
        if self.filename is None:
            raise ValueError("No filename specified for saving schema.")
        if self.schema is None:
            raise ValueError("No schema to save.")
        
        try:
            with open(self.filename, 'w') as file:
                json.dump(self.schema, file, indent=2)
        except Exception as e:
            raise RuntimeError(f"Error saving schema to '{self.filename}': {e}")

    def validate(self, point: Point) -> bool:
        """Validate a Point object against the schema."""
        if self.schema is None:
            raise ValueError("No schema available for validation.")
        
        # Check if the number of data elements matches schema length
        if len(point.data) != len(self.schema):
            raise ValueError(f"Point data length ({len(point.data)}) does not match schema length ({len(self.schema)}).")

        assert isinstance(point.point.time, float) or isinstance(point.point.time, int), "Time should be a float or int."
        
        for i, vals in enumerate(point.data):
            _type = self.schema[i]
            if _type == "string":
                if not isinstance(vals, str):
                    raise ValueError(f"Value {vals} at index {i} is not a string.")
            elif _type == "int":
                if not isinstance(vals, int) or isinstance(vals, bool):  # bool is subclass of int in Python
                    raise ValueError(f"Value {vals} at index {i} is not an integer.")
            elif _type == "float":
                if not isinstance(vals, (int, float)) or isinstance(vals, bool):
                    raise ValueError(f"Value {vals} at index {i} is not a float.")
            elif _type == "bool":
                if not isinstance(vals, bool):
                    raise ValueError(f"Value {vals} at index {i} is not a boolean.")
            else:
                raise ValueError(f"Invalid type '{_type}' at index {i}.")
        
        return True

    def update_schema(self, new_schema: dict):
        """Update the current schema."""
        self.schema = new_schema

    def get_schema(self) -> dict:
        """Get the current schema."""
        return self.schema.copy() if self.schema else None