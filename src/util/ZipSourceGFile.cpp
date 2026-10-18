#include "util/ZipSourceGFile.h"

#include <zip.h>
#include <zipconf.h>

#include "util/Assert.h"  // for xoj_assert
#include "util/i18n.h"    // for FS, _F
#include "util/safe_casts.h"

#include "filesystem.h"  // for path


namespace xoj::util {
zip_int64_t ZipSourceGFile::zipCallback(void* userdata, void *data, zip_uint64_t len, zip_source_cmd_t cmd) {
        ZipSourceGFile* self = static_cast<ZipSourceGFile*>(userdata);

        switch (cmd) {
            // case ZIP_SOURCE_AT_EOF:
            //     return buffer_at_eof(ctx->in);

            // case ZIP_SOURCE_BEGIN_WRITE:
            //     if ((ctx->out = buffer_new(NULL, 0, 0, &ctx->error)) == NULL) {
            //         return -1;
            //     }
            //     ctx->out->offset = 0;
            //     ctx->out->current_fragment = 0;
            //     return 0;

            case ZIP_SOURCE_ACCEPT_EMPTY:
                return 0;

            // case ZIP_SOURCE_BEGIN_WRITE_CLONING:
            //     if ((ctx->out = buffer_clone(ctx->in, len, &ctx->error)) == NULL) {
            //         return -1;
            //     }
            //     ctx->out->offset = len;
            //     ctx->out->current_fragment = ctx->out->nfragments;
            //     return 0;

            case ZIP_SOURCE_CLOSE:
                return 0;

            // case ZIP_SOURCE_COMMIT_WRITE:
            //     buffer_free(ctx->in);
            //     ctx->in = ctx->out;
            //     ctx->out = NULL;
            //     return 0;

            case ZIP_SOURCE_ERROR:
                return zip_error_to_data(&self->ze, data, len);

            case ZIP_SOURCE_FREE:
                return 0;

            case ZIP_SOURCE_GET_FILE_ATTRIBUTES: {
                zip_file_attributes_t *attrs = ZIP_SOURCE_GET_ARGS(zip_file_attributes_t, data, len, &self->ze);
                if(!attrs) {
                    return -1;
                }
                zip_file_attributes_init(attrs);
                attrs->valid = 0U;
                return sizeof(attrs);

                // struct zip_file_attributes {
                //     zip_uint64_t valid;                     /* which fields have valid values */
                //     zip_uint8_t version;                    /* version of this struct, currently 1 */
                //     zip_uint8_t host_system;                /* host system on which file was created */
                //     zip_uint8_t ascii;                      /* flag whether file is ASCII text */
                //     zip_uint8_t version_needed;             /* minimum version needed to extract file */
                //     zip_uint32_t external_file_attributes;  /* external file attributes (host-system specific) */
                //     zip_uint16_t general_purpose_bit_flags; /* general purpose big flags, only some bits are honored */
                //     zip_uint16_t general_purpose_bit_mask;  /* which bits in general_purpose_bit_flags are valid */
                // };
            }

            case ZIP_SOURCE_OPEN:
                // ctx->in->offset = 0;
                // ctx->in->current_fragment = 0;
                return 0;

            case ZIP_SOURCE_READ:
                return self->stream.read(static_cast<char*>(data), len);

            // case ZIP_SOURCE_REMOVE: {
            //     buffer_t *empty = buffer_new(NULL, 0, 0, &ctx->error);
            //     if (empty == NULL) {
            //         return -1;
            //     }
            //
            //     buffer_free(ctx->in);
            //     ctx->in = empty;
            //     return 0;
            // }

            // case ZIP_SOURCE_ROLLBACK_WRITE:
            //     buffer_free(ctx->out);
            //     ctx->out = NULL;
            //     return 0;

            case ZIP_SOURCE_SEEK: {
                auto seekargs = ZIP_SOURCE_GET_ARGS(zip_source_args_seek, data, len, nullptr);
                return self->stream.seek(seekargs->offset, seekargs->whence);
            }
            // case ZIP_SOURCE_SEEK_WRITE:
            //     return buffer_seek(ctx->out, data, len, &ctx->error);

            case ZIP_SOURCE_STAT: {
                zip_stat_t *st = ZIP_SOURCE_GET_ARGS(zip_stat_t, data, len, &self->ze);
                if(!st) {
                    return -1;
                }
                zip_stat_init(st);
                auto info = self->stream.getFileInfo();
                st->mtime = strict_cast<time_t>(info.mtime);
                st->size = info.size;
                // st->comp_size = st->size;
                // st->comp_method = ZIP_CM_STORE;
                // st->encryption_method = ZIP_EM_NONE;
                st->valid = ZIP_STAT_MTIME | ZIP_STAT_SIZE; // | ZIP_STAT_COMP_SIZE | ZIP_STAT_COMP_METHOD | ZIP_STAT_ENCRYPTION_METHOD;

                return sizeof(*st);

                // struct zip_stat {
                //     zip_uint64_t valid;             /* which fields have valid values */
                //     const char *_Nullable name;     /* name of the file */
                //     zip_uint64_t index;             /* index within archive */
                //     zip_uint64_t size;              /* size of file (uncompressed) */
                //     zip_uint64_t comp_size;         /* size of file (compressed) */
                //     time_t mtime;                   /* modification time */
                //     zip_uint32_t crc;               /* crc of file data */
                //     zip_uint16_t comp_method;       /* compression method used */
                //     zip_uint16_t encryption_method; /* encryption method used */
                //     zip_uint32_t flags;             /* reserved for future use */
                // };


            }

            case ZIP_SOURCE_SUPPORTS:
                return zip_source_make_command_bitmap(
                    ZIP_SOURCE_GET_FILE_ATTRIBUTES,
                    ZIP_SOURCE_ACCEPT_EMPTY,
                    ZIP_SOURCE_OPEN,
                    ZIP_SOURCE_READ,
                    ZIP_SOURCE_CLOSE,
                    ZIP_SOURCE_STAT,
                    ZIP_SOURCE_ERROR,
                    ZIP_SOURCE_FREE,
                    ZIP_SOURCE_SEEK,
                    ZIP_SOURCE_TELL,
                    // ZIP_SOURCE_BEGIN_WRITE, ZIP_SOURCE_BEGIN_WRITE_CLONING, ZIP_SOURCE_COMMIT_WRITE, ZIP_SOURCE_REMOVE,
                    // ZIP_SOURCE_ROLLBACK_WRITE, ZIP_SOURCE_SEEK_WRITE, ZIP_SOURCE_TELL_WRITE, ZIP_SOURCE_WRITE, ZIP_SOURCE_SUPPORTS_REOPEN,
                    -1);

            case ZIP_SOURCE_TELL:
                return self->stream.tell();


            // case ZIP_SOURCE_TELL_WRITE:
            //     if (ctx->out->offset > ZIP_INT64_MAX) {
            //         zip_error_set(&ctx->error, ZIP_ER_TELL, EOVERFLOW);
            //         return -1;
            //     }
            //     return (zip_int64_t)ctx->out->offset;
            //
            // case ZIP_SOURCE_WRITE:
            //     if (len > ZIP_INT64_MAX) {
            //         zip_error_set(&ctx->error, ZIP_ER_INVAL, 0);
            //         return -1;
            //     }
            //     return buffer_write(ctx->out, data, len, &ctx->error);

            default:
                zip_error_set(&self->ze, ZIP_ER_OPNOTSUPP, 0);
                return -1;
        }

}

ZipSourceGFile::ZipSourceGFile() { zip_error_init(&this->ze); }

void ZipSourceGFile::open(xoj::FilePath path) {
    this->stream.open(path);

    zip_error_init(&this->ze);
    auto* src = zip_source_function_create(zipCallback, this, &this->ze);
    if (!src) {
        throw std::runtime_error{std::string("unable to create zip_source: ") + zip_error_strerror(&this->ze)};
    }
    this->zip = zip_open_from_source(src, ZIP_RDONLY, &this->ze);
    if (!this->zip) {
        // Most likely the stream is not a zip file
        zip_source_free(src);
    }
}
ZipSourceGFile::~ZipSourceGFile() {
    this->close();
    zip_error_fini(&this->ze);
}

void ZipSourceGFile::close() {
    if (this->zip) {
        if (zip_close(this->zip) != 0) {
            // zip_get_error(this->zip);
            zip_discard(this->zip);
        }
        this->zip = nullptr;
    }
    stream.close();
}
}  // namespace xoj::util
